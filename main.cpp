#include <switch.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

static constexpr size_t MAX_CODE_BYTES = 24000;
static std::string code =
"# SwitchPy IDE\n"
"# Write Python here. Save with X.\n"
"print('Hello from SwitchPy!')\n";
static std::string status = "A edit | X save | Y open | R new line | B delete | L run* | PLUS exit";
static std::string currentPath = "sdmc:/switch/SwitchPyIDE/main.py";
static int selectedLine = 0, firstVisibleLine = 0;

static std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> out; size_t start = 0;
    for (;;) {
        size_t p = s.find('\n', start);
        if (p == std::string::npos) { out.push_back(s.substr(start)); break; }
        out.push_back(s.substr(start, p-start)); start = p+1;
    }
    if (out.empty()) out.push_back("");
    return out;
}
static std::string joinLines(const std::vector<std::string>& lines) {
    std::string s;
    for (size_t i=0;i<lines.size();++i) { s += lines[i]; if(i+1<lines.size()) s+='\n'; }
    return s;
}
static bool keyword(const std::string& w) {
    static const char* words[]={"and","as","assert","async","await","break","class","continue","def","del","elif","else","except","False","finally","for","from","global","if","import","in","is","lambda","None","nonlocal","not","or","pass","raise","return","True","try","while","with","yield","print","range","len","str","int","float","list","dict"};
    for (auto w0:words) if(w==w0) return true; return false;
}
static void highlighted(const std::string& s) {
    bool inString=false; char quote=0;
    for(size_t i=0;i<s.size();) {
        char c=s[i];
        if(!inString && c=='#') { printf("\x1b[90m%s\x1b[0m",s.substr(i).c_str()); return; }
        if(c=='\''||c=='"') {
            if(!inString){inString=true;quote=c;printf("\x1b[32m%c",c);i++;continue;}
            if(quote==c){printf("%c\x1b[0m",c);inString=false;i++;continue;}
        }
        if(inString){putchar(c);i++;continue;}
        if((c>='A'&&c<='Z')||(c>='a'&&c<='z')||c=='_'){
            size_t j=i+1;while(j<s.size()){char d=s[j];if(!((d>='A'&&d<='Z')||(d>='a'&&d<='z')||(d>='0'&&d<='9')||d=='_'))break;j++;}
            std::string w=s.substr(i,j-i); if(keyword(w))printf("\x1b[36m%s\x1b[0m",w.c_str());else printf("%s",w.c_str());i=j;
        } else {putchar(c);i++;}
    }
}
static void saveFile() {
    FILE* f=fopen(currentPath.c_str(),"wb");
    if(!f){status="Save failed. Check SD card.";return;}
    size_t n=fwrite(code.data(),1,code.size(),f);fclose(f);
    status=n==code.size()?"Saved: "+currentPath:"Save failed.";
}
static void openFile() {
    SwkbdConfig kbd; if(R_FAILED(swkbdCreate(&kbd,0))){status="Keyboard error.";return;}
    char path[256];snprintf(path,sizeof(path),"%s",currentPath.c_str());
    swkbdConfigMakePresetDefault(&kbd);swkbdConfigSetHeaderText(&kbd,"Open Python file");
    Result rc=swkbdShow(&kbd,path,sizeof(path));swkbdClose(&kbd);
    if(R_FAILED(rc)||!path[0]){status="Open cancelled.";return;}
    FILE* f=fopen(path,"rb");if(!f){status="Could not open file.";return;}
    std::string data;char buf[1024];
    while(size_t n=fread(buf,1,sizeof(buf),f)){if(data.size()+n>MAX_CODE_BYTES){fclose(f);status="File too large (24 KB limit).";return;}data.append(buf,n);}
    fclose(f);code=data;currentPath=path;selectedLine=0;status="Opened: "+currentPath;
}
static void editLine() {
    auto lines=splitLines(code);selectedLine=std::max(0,std::min(selectedLine,(int)lines.size()-1));
    SwkbdConfig kbd;if(R_FAILED(swkbdCreate(&kbd,0))){status="Keyboard error.";return;}
    char input[1024];snprintf(input,sizeof(input),"%s",lines[selectedLine].c_str());
    swkbdConfigMakePresetDefault(&kbd);swkbdConfigSetHeaderText(&kbd,"Edit selected line");
    Result rc=swkbdShow(&kbd,input,sizeof(input));swkbdClose(&kbd);
    if(R_FAILED(rc)){status="Edit cancelled.";return;}
    if(code.size()-lines[selectedLine].size()+strlen(input)>MAX_CODE_BYTES){status="24 KB limit reached.";return;}
    lines[selectedLine]=input;code=joinLines(lines);status="Line updated.";
}
static void addLine() {
    auto lines=splitLines(code);if(code.size()+1>MAX_CODE_BYTES){status="24 KB limit reached.";return;}
    int at=std::max(0,std::min(selectedLine+1,(int)lines.size()));lines.insert(lines.begin()+at,"");
    code=joinLines(lines);selectedLine=at;status="Blank line added.";
}
static void deleteLine() {
    auto lines=splitLines(code);
    if(lines.size()<=1){code="";selectedLine=0;status="Line cleared.";return;}
    if(selectedLine>=0&&selectedLine<(int)lines.size())lines.erase(lines.begin()+selectedLine);
    selectedLine=std::max(0,std::min(selectedLine,(int)lines.size()-1));code=joinLines(lines);status="Line deleted.";
}
static void runPython() { status="Python runtime not integrated yet; Run is a placeholder."; }
static void draw() {
    consoleClear();auto lines=splitLines(code);
    printf("\x1b[1;36mSwitchPy IDE 0.1.0\x1b[0m | Python editor\nFile: %s\n",currentPath.c_str());
    printf("--------------------------------------------------------------------------\n");
    int visible=17;
    if(selectedLine<firstVisibleLine)firstVisibleLine=selectedLine;
    if(selectedLine>=firstVisibleLine+visible)firstVisibleLine=selectedLine-visible+1;
    for(int i=firstVisibleLine;i<(int)lines.size()&&i<firstVisibleLine+visible;i++){
        if(i==selectedLine)printf("\x1b[7m");
        printf("%3d | ",i+1);highlighted(lines[i]);if(i==selectedLine)printf("\x1b[0m");putchar('\n');
    }
    printf("--------------------------------------------------------------------------\n%s\n",status.c_str());
    printf("UP/DOWN select | A edit | R new line | B delete | X save | Y open | L run* | PLUS exit\n");
    printf("*Execution needs a Switch-compatible Python runtime.\n");
}
int main(int argc,char* argv[]) {
    consoleInit(NULL);fsdevMountSdmc();mkdir("sdmc:/switch/SwitchPyIDE",0777);
    while(appletMainLoop()){
        hidScanInput();u64 down=hidKeysDown(CONTROLLER_P1_AUTO);
        if(down&KEY_PLUS)break;
        if(down&KEY_DUP)selectedLine=std::max(0,selectedLine-1);
        if(down&KEY_DDOWN)selectedLine=std::min((int)splitLines(code).size()-1,selectedLine+1);
        if(down&KEY_A)editLine();if(down&KEY_R)addLine();if(down&KEY_B)deleteLine();
        if(down&KEY_X)saveFile();if(down&KEY_Y)openFile();if(down&KEY_L)runPython();
        draw();consoleUpdate(NULL);
    }
    fsdevUnmountAll();consoleExit(NULL);return 0;
}
