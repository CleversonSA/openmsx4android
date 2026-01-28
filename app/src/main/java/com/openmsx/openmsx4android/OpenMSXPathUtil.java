package com.openmsx.openmsx4android;

import android.content.Context;
import java.io.File;

public final class OpenMSXPathUtil {

    private OpenMSXPathUtil() {}

    public static File getOpenmsxShareDir(Context context) {
        File[] mediaDirs = context.getExternalMediaDirs();

        File base = null;
        if (mediaDirs != null && mediaDirs.length > 0) {
            base = mediaDirs[0]; // geralmente: /storage/emulated/0/Android/media/<package>/
        }

        if (base == null) {
            // Fallback (menos amigável para usuário, mas sempre funciona)
            base = context.getExternalFilesDir(null); // /storage/emulated/0/Android/data/<package>/files
        }

        File shareDir = new File(base, "openmsx/share");
        if (!shareDir.exists() && !shareDir.mkdirs()) {
            // Se falhar criar, ainda retorna o File, mas você pode logar/lançar exceção
        }
        return shareDir;
    }

    public static String getOpenmsxSharePath(Context context) {
        return getOpenmsxShareDir(context).getAbsolutePath();
    }
}
