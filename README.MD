# OPENMSX4ANDROID

[English](README.en.md) \| [PT-BR](README.md)

Um port não oficial para Android do quase perfeito emulador de MSX
disponível.

# AVISO LEGAL

Não sou o proprietário do projeto OpenMSX (https://openmsx.org/), sou
apenas um entusiasta de MSX que ama esse quase perfeito emulador para
emular máquinas brasileiras (MSX Expert, Sharp HotBit).

Utilizei os arquivos fonte do projeto OpenMSX como uma grande biblioteca
para permitir sua execução na maioria das plataformas Android modernas,
e fiz alguns patches que devolverei à comunidade para ajudar na criação
de uma versão oficial para Android.

Dito isso, esteja avisado:

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...IN NO
EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE
```

Sempre faça um backup de segurança do seu dispositivo. É um projeto open
source desenvolvido no tempo livre. Todos os problemas, riscos e danos
são de sua responsabilidade! :)

# SOBRE O PROJETO

Este port foi iniciado após eu criar uma imagem OpenMSX 21 com alguns
addons para a plataforma Raspberry PI (veja meu repositório:
https://github.com/CleversonSA/uosxpi).

O OpenMSX já teve um port oficial para Android, mas está desatualizado,
a versão 21 trouxe otimizações que o port antigo não estava preparado
para suportar.

Dispositivos Android são mais amplamente disponíveis do que Raspberry
Pis, e na comunidade brasileira de MSX no Facebook
(https://www.facebook.com/groups/msxbrasiloficial/?locale=pt_BR) a ideia
de um port do OpenMSX foi muito bem aceita!

Eu possuo hardware MSX real, mas penso em oferecer para quem não quer
gastar muito dinheiro com hardware antigo e raro, trazendo nostalgia e
também para quem nunca utilizou a plataforma MSX. Carregar um "MSX" no
bolso, disponível a qualquer momento, seja para jogos ou programação em
BASIC, para mim é incrível!

Gastei cerca de 2 meses até a versão beta, utilizando IA para me ajudar
a entender profundamente a arquitetura do OpenMSX (especialmente como
migrar os scripts OpenGL ES para a versão que o Android entende),
compilação via Android NDK, compilação de bibliotecas extras necessárias
(tcl, SDL, ImGUI, etc) e assim por diante.

Este não é o projeto final, pois utilizei a versão quase recente do
OpenMSX do repositório oficial (Versão 21 de dez 2025). Continuarei
atualizando sempre que possível.

Divirtam-se com os **"fudebas"!** :)

# VERSÕES ANDROID SUPORTADAS

ATENÇÃO: Para a versão beta (fev 2026), tive que usar o NDK do Android
14. Portanto, dispositivos mais antigos, tablets mais antigos, podem não
    aceitar o arquivo APK.

Verifique se seu dispositivo Android é Android 14 ou mais recente. Para
desenvolvimento utilizei um tablet Android GO (14) de marca simples e um
Redmi Note 14, ambos funcionaram muito bem.

No futuro, conforme adquirir mais experiência em desenvolvimento Android
NDK, poderei tentar versões NDK mais antigas para dar nova vida a
tablets ou smartphones antigos.

# INSTALAÇÃO DO APK (SEM ROOT!)

### Versão Beta

Atenção: Este projeto ainda não está disponível na Google Play Store,
até finalizar a versão beta deste port. Portanto, você precisa enviar
manualmente o arquivo APK para o dispositivo e confiar na instalação a
partir desta fonte. Não é necessário ROOT ou permissões especiais,
apenas um aplicativo padrão.

-   Baixe o "openmsx4android.apk" mais recente da pasta "/dist"
-   Envie o arquivo para o dispositivo via cabo USB (não é necessário
    modo desenvolvedor). Salve na pasta "Download", por exemplo.
-   No dispositivo Android, acesse o aplicativo Arquivos, vá até a pasta
    "Download" e toque no arquivo "openmsx4android"
-   Instale o emulador confiando na origem deste projeto
    (com.openmsx4android).
-   Aguarde alguns segundos (+- 250MB necessários apenas para o APK)
-   Volte para a tela inicial e toque no aplicativo "openmsx4android".
    Nesta versão beta (2026-02-09), ele possui o ícone padrão do
    Android. Alterarei para o ícone padrão do OpenMSX posteriormente.
-   IMPORTANTE!! -\> Aguarde alguns segundos para o processo de
    descompactação dos arquivos de assets do OpenMSX terminar (10-30s).
    Esse processo ocorre apenas uma vez, não se preocupe.

Se tudo ocorrer corretamente, você verá o menu do OpenMSX 21 e a tela do
C-BIOS.

# ARQUIVO CMDLINE.TXT (MEU RECURSO)

Como é um APK Android, você não possui um script launcher como em
instalações Linux ou Windows, então como colocar opções de configuração,
escolha de máquina, associação de discos, etc?

No port, antes de iniciar o motor do OpenMSX verifico se o arquivo
`cmdline.txt` está presente:

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share/cmdline.txt
```

Este arquivo não é um batch, mas contém tokens para chamar o openmsx
passando parâmetros, por exemplo:

```
-machine "Gradiente_Expert_GPC-1"
-command "set fullscreen on"
-command ;"set scale_factor 2"
-command "set mute off"
-command "set blur 0"
-command "set auto_enable_reverse off"
-exta Musical_Memory_Mapper
-extb slotexpander
-ext DDX_3.0 -ext nextor
```

Assim posso iniciar o openmsx com configurações personalizadas.

O `cmdline.txt` não está presente na instalação padrão. Se quiser
configurações personalizadas, você deve criar um novo em:

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share
```

# INSTALANDO SYSTEMROMS E EXTENSÕES

Atenção: A instalação do APK NÃO possui arquivos de ROM de hardware ou
jogos, apenas C-BIOS, devido a restrições de copyright.

Todos os arquivos de assets do OpenMSX (pasta *share*) são instalados
em:

```
Internal Storage/Android/media/com.openmsx.openmsx4android/openmsx/share
```
ou mais especificamente

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share
```

Você pode colocar seus arquivos de ROM de hardware, extensões, etc nessa
pasta da mesma forma que faria em uma instalação no PC.

Não é necessário jailbreak/root/modo desenvolvedor para colocar
arquivos.

# O QUE É SUPORTADO

-   Teclado Virtual Android Nativo
-   Teclado físico (USB/BT)
-   Gerenciador de Disco
-   Menus GUI nativos OpenMSX 21
-   Pasta como disco (funciona, não foi afetado)
-   Som
-   Rotação de tela (OBS: Manual, você deve iniciar a aplicação com a tela deitada ou em pé)

# O QUE NÃO É SUPORTADO (AINDA)

-   Sem suporte a Joystick
-   Sem pastas de usuário openMSX (como "persistent")
-   Sem teclas de função overlay
-   Sem suporte a cartão SD para pasta share
-   Ícones overlay e GUI podem melhorar
-   Sem suporte a LaserDisc
-   Sem suporte a screenshot

# COMPILANDO OS FONTES

Este projeto foi compilado usando Windows 10 Pro utilizando:

-   Android Studio 2025
-   OpenJDK 8
-   Android 14 e NDK 29

Se você possui o Android Studio totalmente funcional, com caminhos Java
e NDK funcionando, basta clonar o projeto e importar os arquivos na IDE.

Atenção: Como é um projeto GRANDE, a primeira compilação pode levar
40min-1 hora (uso i5 7ª geração 16GB RAM).

# CONTRIBUINDO

É apenas um projeto por diversão. Gosto de ter no meu smartphone ou
tablet uma máquina MSX Gradiente Expert ou Sharp Hotbit.

Decidi manter aberto para contribuições para melhorar este port e
futuramente ajudar a comunidade OpenMSX a construir novamente um port
oficial Android do emulador.

Este release usa a versão fechada mais recente de Nov/2025 do OpenMSX
21.

Como disse, OpenMSX foi utilizado como uma grande biblioteca, e algumas
classes foram modificadas com implementação de stubs (veja
`src/main/cpp/openmsx_android` e `src/main/cpp/openmsx_generated`), e
modifiquei meu CMakeLists.txt para ignorar a classe principal do OpenMSX
e usar a minha (veja `src/main.cpp`), onde tudo começou.

Para tornar possível rodar o emulador na plataforma Android, usando
ANativeWindow, o port possui uma camada Java criada a partir do template
SDL2 (veja `src/main/java/org/libsdl/app/OpenMSX4AndroidSDLActivity`)
como "cola".

# PROBLEMAS CONHECIDOS

-   Às vezes teclas como backspace e enter ficam pressionadas para
    sempre no teclado virtual
-   Menu ImGUI muito pequeno em dispositivos com DPI alto
-   Às vezes perco conteúdo de arquivos BASIC após sair e voltar ao App
-   Ao instalar o APK, a Galeria de Fotos mostra vários assets do
    OpenMSX
-   Se criar cmdline.txt incorreto o OpenMSX pode travar ao iniciar
-   Apenas Android 14+ suportado
-   Às vezes teclado virtual e botões overlay não respondem (problema de
    foco)

# CHANGELOG

## 2026-02-20 release
-  Suporte a rotação manual de tela. 

## 2026-02-13 release

-   Melhorado bastante suporte ao Teclado Virtual Android
-   Corrigido suporte SHIFT e CAPS lock
-   Removidos comentários PTBR e melhorados comentários padrão

## 2026-02-09 release

-   Versão Alpha, funciona, roda máquinas, extensões, som, teclas, etc,
    mas possui alguns problemas
-   Apenas suporte a tela vertical
-   Suporte básico ao Teclado Virtual Android
-   Suporte OpenGL ES 3.0 (arquivos tex e fragment atualizados, também
    ImGui)
-   Armazenamento do usuário suportado
-   cmdline.txt funcionando como launcher tipo batch
