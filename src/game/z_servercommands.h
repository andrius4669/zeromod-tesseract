#ifndef Z_SERVERCOMMANDS_H
#define Z_SERVERCOMMANDS_H 1

typedef void (* z_scmdfun)(int argc, char **argv, int sender);

struct z_servcmdinfo
{
    string name;
    z_scmdfun fun;
    int privilege;
    int numargs;
    bool hidden;
    
    z_servcmdinfo(): fun(NULL), privilege(PRIV_NONE), numargs(0), hidden(false) { name[0] = '\0'; }
    z_servcmdinfo(const z_servcmdinfo &n): fun(n.fun), privilege(n.privilege),
        numargs(n.numargs), hidden(n.hidden) { copystring(name, n.name); }
    z_servcmdinfo(const char *_name, z_scmdfun _fun, int _priv, int _numargs, bool _hidden): fun(_fun),
        privilege(_priv), numargs(_numargs), hidden(_hidden) { copystring(name, _name); }
    z_servcmdinfo &operator =(const z_servcmdinfo &n)
    {
        if(&n != this)
        {
            copystring(name, n.name);
            fun = n.fun;
            privilege = n.privilege;
            numargs = n.numargs;
            hidden = n.hidden;
        }
        return *this;
    }
    ~z_servcmdinfo() {}
    
    bool valid() { return name[0] && fun; }
};

vector<z_servcmdinfo> z_servcommands;
static bool z_initedservcommands = false;
static vector<z_servcmdinfo> *z_servcommandinits = NULL;

bool addservcmd(const z_servcmdinfo &cmd)
{
    if(!z_initedservcommands)
    {
        if(!z_servcommandinits) z_servcommandinits = new vector<z_servcmdinfo>;
        z_servcommandinits->add(cmd);
        return true;
    }
    z_servcommands.add(cmd);
    return true;
}

static bool z_initservcommands()
{
    z_initedservcommands = true;
    
    if(z_servcommandinits)
    {
        loopv(*z_servcommandinits) if((*z_servcommandinits)[i].valid()) addservcmd((*z_servcommandinits)[i]);
        DELETEP(z_servcommandinits);
    }
    
    return true;
}

bool z_parseclient(const char *str, int *cn)
{
    if(!str) return false;
    char *end;
    int n = strtol(str, &end, 10);
    if(*str && !*end) { *cn = n; return true; }
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcmp(str, ci->name)) { *cn = ci->clientnum; return true; }
    }
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcasecmp(str, ci->name)) { *cn = ci->clientnum; return true; }
    }
    return false;
}

#define SCOMMANDZ(_name, _priv, _funcname, _args, _hidden) UNUSED static bool __s_dummy__##_name = addservcmd(z_servcmdinfo(#_name, _funcname, _priv, _args, _hidden))
#define SCOMMAND(_name, _priv) SCOMMANDZ(_name, _priv, _name, 0, false)
#define SCOMMANDH(_name, _priv) SCOMMANDZ(_name, _priv, _name, 0, true)
#define SCOMMANDN(_name, _priv, _func) SCOMMANDZ(_name, _priv, _func, 0, false)
#define SCOMMANDNH(_name, _priv, _func) SCOMMANDZ(_name, _priv, _func, 0, true)
#define SCOMMANDA(_name, _priv, _args) SCOMMANDZ(_name, _priv, _name, _args, false)
#define SCOMMANDAH(_name, _priv, _args) SCOMMANDZ(_name, _priv, _name, _args, true)
#define SCOMMANDNA(_name, _priv, _func, _args) SCOMMANDZ(_name, _priv, _func, _args, false)
#define SCOMMANDNAH(_name, _priv, _func, _args) SCOMMANDZ(_name, _priv, _func, _args, true)

#endif //Z_SERVERCOMMANDS_H
