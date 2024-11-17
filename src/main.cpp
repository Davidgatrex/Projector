#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>

#define CFILEPARAMETERCOUNT 1

using namespace std;

struct Project
{
    string Name;
    string Lang;
};

string cfgdir = "";
string maincfile = "";
string projectsDir = "";
string projectsDB = "";
string langsDir = "";
std::vector<Project> Projects;
int pcnt = 0;
int lcnt = 0;
Project *CurrentProject = NULL;
vector<string> Langs;

void vim(string path){
    string cmd = "vim " + path;
    system(cmd.c_str());
}

inline bool fexist(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void mkdir(string name)
{
    string b = "mkdir -p ";
    b.append(name);
    system(b.c_str());
}

string exec(string command)
{
    char buffer[128];
    string result = "";

    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return "popen failed!";
    }

    // read till end of process:
    while (!feof(pipe))
    {

        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

    pclose(pipe);
    return result;
}

void setupcfile()
{
    string pdirp = "";
    ofstream f(maincfile);
    cout << "Project directory path: ";
    cin >> pdirp;
    f << pdirp;
    f.close();
}

void Createlang(string name)
{
    mkdir(langsDir + "/" + name);
}

void setupProjectDirectory()
{
    ofstream f(projectsDB);
    f << 0;
    f.close();
}

void initMF()
{
    FILE *f = fopen(maincfile.c_str(), "r");
    if (f == NULL)
    {
        // File doesn't exist
        setupcfile();
        initMF();
    }
    else
    {
        fclose(f);
        string content = "";
        ifstream fi(maincfile);
        string buffer[CFILEPARAMETERCOUNT];
        int lines = 0;
        while (getline(fi, content))
        {
            buffer[lines] = content;
            lines++;
        }
        if (lines < CFILEPARAMETERCOUNT)
        {
            cout << "Modified config file. Defaulting..." << endl;
            fclose(fopen(maincfile.c_str(), "w"));
            setupcfile();
            lines = 0;
            while (getline(fi, content))
            {
                buffer[lines] = content;
                lines++;
            }
        }
        projectsDir = buffer[0];
    }
}

void initDB()
{
    FILE *f = fopen(projectsDB.c_str(), "r");
    if (f == NULL)
    {
        // File doesn't exist
        setupProjectDirectory();
        initDB();
    }
    else
    {
        fclose(f);
        string content = "";
        ifstream fi(projectsDB);
        int cproj = 0;
        bool lid = true;
        while (getline(fi, content))
        {
            if (cproj == 0)
            {
                try{
                    pcnt = std::stoi(content);
                    Projects.resize(pcnt);
                    cproj++;
                }catch (const std::exception &e) {
                    std::cerr << "Error al convertir el número de proyectos: " << e.what() << std::endl; 
                    return; 
                }
            }
            else
            {
                if (lid)
                    Projects[cproj - 1].Name = content;
                else
                {
                    Projects[cproj - 1].Lang = content;
                    cproj++;
                }
                lid = !lid;
            }
        }
        fi.close();
    }
}

void initLangs()
{
    struct stat sb;
    if (stat(langsDir.c_str(), &sb) == 0)
    {
        // Exist
    }
    else
    {
        // Not exist
        mkdir(langsDir);
    }
    string fnm = langsDir;
    fnm.append("/langs.projectorlangs");
    FILE *f = fopen(fnm.c_str(), "r");
    if (f == NULL)
    {
        //File doesn't exist.
        // wf is Write File
        ofstream wf(fnm);
        wf << 0;
        wf.close();
        initLangs();
    }
    else
    {
        fclose(f);
        string content = "";
        ifstream fi(fnm);

        int line = 0;
        while (getline(fi, content))
        {
            if (line == 0)
            {
                Langs = vector<string>();
                lcnt = atoi(content.c_str());
            }
            else
            {
                Langs.resize(Langs.size()+1);
                Langs.at(Langs.size()-1) = content;
                Createlang(content);
            }
            line++;
        }
        fi.close();
    }
}

void Init()
{
    cfgdir = exec("echo $HOME");
    cfgdir.replace(cfgdir.length() - 1, 1, "/");
    cfgdir.append(".projector");
    maincfile = cfgdir;
    maincfile.append("/config.projectorconfig");
    projectsDB = cfgdir;
    projectsDB.append("/projects.projectors");
    langsDir = cfgdir;
    langsDir.append("/langs");
    struct stat sb;
    if (stat(cfgdir.c_str(), &sb) == 0)
    {
        // Exists;
    }
    else
    {
        // Not exist
        cout << "Projector's configuration directory not found. Creating...    ";
        system("mkdir $HOME/.projector");
        cout << "[Done]" << endl;
    }
    initMF();
    initDB();
    initLangs();
}

void projectlist()
{
    cout << "Your project list:" << endl;
    for (int i = 0; i < pcnt; i++)
    {
        cout << i << "-> " << Projects[i].Name << " | " << Projects[i].Lang << endl;
    }
}

void select()
{
    cout << "ID: ";
    string sid = "";
    cin >> sid;
    int id = atoi(sid.c_str());
    CurrentProject = &Projects[id];
}

void New()
{
    string pnam = "";
    cout << "Name: ";
    cin >> pnam;
    for (int i = 0; i < pcnt; i++)
    {
        if (Projects[i].Name == pnam)
        {
            cout << "Project already exist." << endl;
            return;
        }
    }

    string prodir = projectsDir + "/" + pnam;

    string lang = "";
    cout << "Language: ";
    cin >> lang;

    string langdir = langsDir + "/" + lang;

    mkdir(prodir);

    ofstream pdb(projectsDB);
    pdb << (pcnt + 1) << endl;
    for (int i = 0; i < pcnt; i++)
    {
        pdb << Projects[i].Name << endl
            << Projects[i].Lang << endl;
    }
    pdb << pnam << endl
        << lang << endl;
    pdb.close();

    for (int i = 0; i < lcnt; i++)
    {
        if (Langs[i] == lang)
        {
            cout << "Known language. Making project structure... ";
            string langdirf = langdir + "/dirs.prld";
            FILE *F = fopen(langdirf.c_str(), "r");
            if (F != NULL)
            {
                fclose(F);
                string dir = "";
                ifstream dirsf(langdirf);
                while (getline(dirsf, dir))
                {
                    mkdir(prodir + "/" + dir);
                }
                dirsf.close();
            }

            string langcopyf = langdir + "/copy.prlc";
            F = fopen(langcopyf.c_str(), "r");
            if (F != NULL)
            {
                fclose(F);
                string fil = "";
                ifstream copyf(langcopyf);
                while (getline(copyf, fil))
                {
                    ifstream sf(langdir + "/" + fil);
                    ofstream of(prodir + "/" + fil);
                    string cline = "";
                    while (getline(sf, cline))
                    {
                        of << cline << endl;
                    }
                    sf.close();
                    of.close();
                }
                copyf.close();
            }
            cout << "[Done]" << endl;
        }
    }
    cout << "Reinitializing database... ";
    initDB();
    cout << "[Done]" << endl;
}

void listLangs()
{
    cout << "Your languages:" << endl;
    for (int i = 0; i < lcnt; i++)
    {
        cout << (i+1) << "-> " << Langs[i] << endl;
    }
}

void newlang()
{
    cout << "Name: ";
    string lname = "";
    cin >> lname;
    for (int i = 0; i < lcnt; i++)
    {
        if (Langs[i] == lname)
        {
            cout << "Language already registered. Remove it first." << endl;
            return;
        }
    }
    string fnm = langsDir;
    fnm.append("/langs.projectorlangs");
    ofstream f(fnm);
    f << (lcnt + 1) << endl;
    for (int i = 0; i < lcnt; i++)
    {
        f << Langs[i] << endl;
    }
    f << lname;
    f.close();
    string langdir = langsDir + "/" + lname;
    mkdir(langdir);
    {
        string dirsfile = langdir + "/dirs.prld";
        string cmd = "touch " + dirsfile;
        system(cmd.c_str());
        vim(dirsfile);
    }
    {
        string copyfile = langdir + "/copy.prlc"; 
        string cmd = "touch " + copyfile;
        system(cmd.c_str());
        vim(copyfile);
    }
    {
        string copyfile = langdir + "/copy.prlc"; 
        ifstream copyf(copyfile);
        string line = "";
        while(getline(copyf, line)){
            if(line.find("Makefile") <= line.size() || line.find("makefile") <= line.size())
            {
                ofstream of(langdir + "/" + line);
                of << "NAME = ${NAME}" << endl << "#DO NOT MODIFY ABOVE THIS LINE. NAME IS THE NAME OF THE PROGRAM.";
            }else{
                string cmd = "touch " + langdir + "/" + line;
                system(cmd.c_str());
            }
            vim(langdir + "/" + line);
        }
        copyf.close();
    }
    {
        string buildfile = langdir + "/build.sh";
        ofstream stream(buildfile);
        stream << "# Script for building. $NAME is the name of the program." << endl << "cd $PROJECTORCPATH" << endl << "#DO NOT MODIFY ABOVE THIS LINE." << endl;
        stream.close();
        vim(buildfile);
        string cmd = "chmod +x " + buildfile;
        system(cmd.c_str());
    }
    {
        string runfile = langdir + "/run.sh";
        ofstream stream(runfile);
        stream << "# Script for running. $NAME is the project name and $ARGS is the arguments passed to the program." << endl << "cd $PROJECTORCPATH" << endl << "#DO NOT MODIFY ABOVE THIS LINE." << endl;
        stream.close();
        vim(runfile);
        string cmd = "chmod +x " + runfile;
        system(cmd.c_str());
    }
    initLangs();
    listLangs();
}

void remlang(){
    cout << "ID: ";
    string input = "";
    int id = 0;
    cin >> id;
    if(id == 0 || id > lcnt){
        cout << "No language with such ID" << endl;
        return;
    }
    string lname = Langs[id-1];
    string fnm = langsDir;
    fnm.append("/langs.projectorlangs");
    ofstream f(fnm);
    f << (lcnt - 1) << endl;
    for (int i = 0; i < lcnt; i++)
    {
        if(Langs[i] != lname)
            f << Langs[i] << endl;
    }
    f.close();
    string langdir = langsDir + "/" + lname;
    string cmd = "rm -r" + langdir;
    system(cmd.c_str());
    initLangs();
    listLangs();
}

void langCFG()
{
    listLangs();
    for (;;)
    {
        string option = "";
        cout << "$=# ";
        cin >> option;
        if (option == "exit")
            return;
        else if (option == "list")
            listLangs();
        else if (option == "new")
            newlang();
        else if(option == "remove")
            remlang();
        else
        {
            cout << "Unknown command" << endl;
        }
    }
}

void Remproj(){
    cout << "ID: ";
    int id = 0;
    string s = "";
    cin >> s;
    if(s == ""){
        cout << "Cannot be null" << endl;
        return;
    }
    id = atoi(s.c_str());
    if(id < 0 || id > pcnt-1){
        cout << "Out of range" << endl;
        return;
    }
    string pnam = Projects[id].Name;
    string prodir = projectsDir + "/" + pnam;
    ofstream pdb(projectsDB);
    pdb << (pcnt - 1) << endl;
    for (int i = 0; i < pcnt; i++)
    {
        if(i != id)
        pdb << Projects[i].Name << endl
            << Projects[i].Lang << endl;
    }
    pdb.close();
    string c = "cd " + projectsDir + "&& rm -r " + pnam;
    system(c.c_str());
    initDB();
}

void processInput(string in)
{
    if (in == "select")
        select();
    else if (in == "new")
        New();
    else if (in == "list")
        projectlist();
    else if (in == "exit")
        exit(0);
    else if (in == "langcfg")
        langCFG();
    else if(in == "remove")
        Remproj();
    else
    {
        cout << "unknown command" << endl;
    }
}

void Run(string s)
{
    bool found = false;
    for (int i = 0; i < lcnt; i++)
    {
        if (Langs[i] == CurrentProject->Lang)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        string cmd = "(cd " + langsDir + "/" + CurrentProject->Lang + "; export PROJECTORCPATH=" + projectsDir + "/" + CurrentProject->Name + "; export ARGS=" + s + "; export NAME=" + CurrentProject->Name + "; ./run.sh)";
        //cout << "Command is:    " << cmd << endl;
        system(cmd.c_str());
    }
    else
    {
        cout << "Unknown language. Please, run and build manually with: \"shell\". That will launch a new shell." << endl;
    }
}

void Build()
{
    bool found = false;
    for (int i = 0; i < lcnt; i++)
    {
        if (Langs[i] == CurrentProject->Lang)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        string cmd = "(cd " + langsDir + "/" + CurrentProject->Lang + "; export PROJECTORCPATH=" + projectsDir + "/" + CurrentProject->Name + "; export NAME=" + CurrentProject->Name + "; ./build.sh)";
        //cout << "Command is:    " << cmd << endl;
        system(cmd.c_str());
    }
    else
    {
        cout << "Unknown language. Please, run and build manually with: \"shell\". That will launch a new shell." << endl;
    }
}

void projectInput(string in)
{
    if (in == "run")
    {
        string args = "";
        getline(cin, args);
        Run(args);
    }
    else if (in == "build")
        Build();
    else if (in == "exit")
        CurrentProject = NULL;
    else if (in == "shell")
    {
        string cmd = "cd " + projectsDir + "/" + CurrentProject->Name + "&& bash";
        system(cmd.c_str());
    }
    else if (in == "list"){
        string cmd = "cd " + projectsDir + "/" + CurrentProject->Name + " && find";
        system(cmd.c_str());
    }
    else if(in == "edit"){
        string filename = "";
        cout << "Filename: ";
        cin >> filename;
        vim(projectsDir + "/" + CurrentProject->Name + "/" + filename);
    }
    else if(in == "remove"){
        string filename = "";
        cout << "Filename: ";
        cin >> filename;
        string s = "rm " + projectsDir + "/" + CurrentProject->Name + "/" + filename;
        system(s.c_str());
    }
    else if(in == "remdir"){
        string filename = "";
        cout << "Directory name: ";
        cin >> filename;
        string s = "rmdir " + projectsDir + "/" + CurrentProject->Name + "/" + filename;
        system(s.c_str());
    }
    else if(in == "makedir"){
        string filename = "";
        cout << "Directory name: ";
        cin >> filename;
        string s = "mkdir " + projectsDir + "/" + CurrentProject->Name + "/" + filename;
        system(s.c_str());
    }
    else if(in == "rename"){
        string filename = "";
        string newname = "";
        cout << "Name: ";
        cin >> filename;
        cout << "New name: ";
        cin >> newname;
        string s = "mv " + projectsDir + "/" + CurrentProject->Name + "/" + filename + " " + projectsDir + "/" + CurrentProject->Name + "/" + newname;
        system(s.c_str());
    }
    else
    {
        cout << "Unknown command" << endl;
    }
}

void ishell()
{
    for (;;)
    {
        string input = "";
        if (CurrentProject != NULL)
        {
            cout << "[" + CurrentProject->Name + "]";
        }
        cout << "~~>";
        cin >> input;
        if (CurrentProject == NULL)
            processInput(input);
        else
            projectInput(input);
    }
}

int main(int argc, char const *argv[])
{
    Init();
    projectlist();
    ishell();
    return 0;
}
