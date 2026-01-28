//
// Created by cleve on 16/12/2025.
//
#include <android/log.h>
#include <SDL.h>
#include "openmsx_android/openmsx_entry.h"
#include "build-info.hh"
#include <sys/stat.h>
#include <string>
#include <vector>
#include <jni.h>
#include <fstream>
#include <sstream>
#include <cctype>
#include <regex>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "MVP0", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MVP0", __VA_ARGS__)


    extern "C" JNIEXPORT void JNICALL
    Java_org_libsdl_app_OpenMSX4AndroidSDLActivity_nativeSetSharePath(
            JNIEnv *env, jclass /*clazz*/, jstring jpath) {
        const char *path = env->GetStringUTFChars(jpath, nullptr);

        env->ReleaseStringUTFChars(jpath, path);
        size_t length = std::strlen(path);
        char *destination = new char[length + 1];
        std::strcpy(destination, path);
        openmsx::DATADIR = destination;
    }


    static inline void trimInPlace(std::string &s) {
        auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        while (!s.empty() && !notSpace((unsigned char) s.front())) s.erase(s.begin());
        while (!s.empty() && !notSpace((unsigned char) s.back())) s.pop_back();
    }


// Remove comment lines
    static std::string stripCommentsOutsideQuotes(const std::string &line) {
        bool inQuotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"' && (i == 0 || line[i - 1] != '\\')) inQuotes = !inQuotes;

            if (!inQuotes) {
                if (c == '#') {
                    return line.substr(0, i);
                }
                if (c == '/' && i + 1 < line.size() && line[i + 1] == '/') {
                    return line.substr(0, i);
                }
            }
        }
        return line;
    }


// Tokenize like shell light style
// - keep apart by spaces
// - respect double quotes
// - accept single quotes inside double quotes
    static std::vector<std::string> tokenizeCommandLine(const std::string &rawLine) {
        std::vector<std::string> out;
        std::string line = stripCommentsOutsideQuotes(rawLine);
        trimInPlace(line);
        if (line.empty()) return out;

        std::string cur;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];

            if (inQuotes) {
                if (c == '\\' && i + 1 < line.size()) {
                    char n = line[i + 1];
                    if (n == '"' || n == '\\') { // supported scapes
                        cur.push_back(n);
                        ++i;
                        continue;
                    }
                    // unknown scape, keep the current dashes
                    cur.push_back(c);
                    continue;
                }
                if (c == '"') {
                    inQuotes = false;
                    continue;
                }
                cur.push_back(c);
                continue;
            } else {
                if (std::isspace((unsigned char) c)) {
                    if (!cur.empty()) {
                        out.push_back(cur);
                        cur.clear();
                    }
                    continue;
                }
                if (c == '"') {
                    inQuotes = true;
                    continue;
                }
                cur.push_back(c);
            }
        }

        if (!cur.empty()) out.push_back(cur);

        return out;
    }


    static void mkdirs(const std::string &fullPath) {
        // cria diretórios para um *arquivo* (até o último '/')
        size_t pos = 0;
        while ((pos = fullPath.find('/', pos + 1)) != std::string::npos) {
            std::string dir = fullPath.substr(0, pos);
            mkdir(dir.c_str(), 0700); // ok se já existir
        }
    }


    static std::vector<std::string> loadArgsFromCmdlineFile(const std::string &datadir) {
        std::vector<std::string> args;
        args.emplace_back("openmsx"); // fixed command line

        std::string path = datadir;
        if (!path.empty() && path.back() != '/' && path.back() != '\\') path.push_back('/');
        path += "cmdline.txt";

        std::ifstream f(path);
        if (!f.is_open()) {
            return args;
        }

        std::string line;
        while (std::getline(f, line)) {
            auto tokens = tokenizeCommandLine(line);
            for (auto &t: tokens) args.push_back(std::move(t));
        }

        return args;
    }


    static bool copyRW(SDL_RWops *in, SDL_RWops *out) {
        char buf[64 * 1024];
        size_t r;
        while ((r = SDL_RWread(in, buf, 1, sizeof(buf))) > 0) {
            if (SDL_RWwrite(out, buf, 1, r) != r) return false;
        }
        return true;
    }

    static std::vector<std::string> readLinesFromAsset(const char *assetPath) {
        std::vector<std::string> lines;
        SDL_RWops *rw = SDL_RWFromFile(assetPath, "rb");
        if (!rw) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                "Failed to open asset %s: %s", assetPath, SDL_GetError());
            return lines;
        }
        Sint64 sz = SDL_RWsize(rw);
        if (sz <= 0) {
            SDL_RWclose(rw);
            return lines;
        }

        std::string content;
        content.resize(size_t(sz));
        SDL_RWread(rw, content.data(), 1, size_t(sz));
        SDL_RWclose(rw);

        size_t start = 0;
        while (start < content.size()) {
            size_t end = content.find('\n', start);
            if (end == std::string::npos) end = content.size();
            std::string line = content.substr(start, end - start);
            // trim CR
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty() && line[0] != '#') lines.push_back(line);
            start = end + 1;
        }
        return lines;
    }

    static bool extractOpenmsxAssets(const std::string &systemDir) {
        // systemDir ex: /data/data/<pkg>/files/openmsx
        auto files = readLinesFromAsset("openmsx/index.txt");
        if (files.empty()) return false;

        for (const auto &rel: files) {
            std::string asset = "openmsx/" + rel;
            std::string out = systemDir + "/" + std::regex_replace(rel, std::regex("share/"), "");

            // já existe? pule
            SDL_RWops *test = SDL_RWFromFile(out.c_str(), "rb");
            if (test) {
                SDL_RWclose(test);
                continue;
            }

            SDL_RWops *in = SDL_RWFromFile(asset.c_str(), "rb");
            if (!in) {
                __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                    "Missing asset %s: %s", asset.c_str(), SDL_GetError());
                return false;
            }

            mkdirs(out);
            SDL_RWops *outRw = SDL_RWFromFile(out.c_str(), "wb");
            if (!outRw) {
                SDL_RWclose(in);
                __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                    "Can't create %s: %s", out.c_str(), SDL_GetError());
                return false;
            }

            bool ok = copyRW(in, outRw);
            SDL_RWclose(in);
            SDL_RWclose(outRw);

            if (!ok) {
                __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                    "Copy failed for %s", rel.c_str());
                return false;
            }
        }
        return true;
    }

    extern "C" int SDL_main(int argc, char **argv) {
        (void) argc;
        (void) argv;

        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"); // inofensivo fora do Linux

        SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        char *pref = SDL_GetPrefPath("com.openmsx", "openmsx4android");
        std::string prefPath = pref ? pref : "";
        SDL_free(pref);

        extractOpenmsxAssets(openmsx::DATADIR);

        //args = {"openmsx","-machine","Gradiente_Expert_GPC-1", "-command","set fullscreen on"};
        std::vector<std::string> args = loadArgsFromCmdlineFile(openmsx::DATADIR);

        if (args.size() == 1) {
            args = {"openmsx", "-command", "set fullscreen on"};
        }
        std::vector<char *> cargs;
        cargs.reserve(args.size());
        for (auto &s: args) cargs.push_back(s.data());


        /*std::string test = systemDir + "/share/machines/C-BIOS_MSX2+.xml";
        SDL_RWops* rw = SDL_RWFromFile(test.c_str(), "rb");
        __android_log_print(ANDROID_LOG_INFO, "openmsx4android", "test path=%s exists=%d",
                            test.c_str(), rw != nullptr);
        if (rw) SDL_RWclose(rw);
         */

        return openmsx_android_main((int) cargs.size(), cargs.data());

    }