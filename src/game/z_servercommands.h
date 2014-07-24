#ifndef Z_SERVERCOMMANDS_H
#define Z_SERVERCOMMANDS_H

typedef void (* z_scmdfun)(int argc, char **argv, int sender);

struct z_servcmdinfo
{
    string name;
    z_scmdfun fun;
    int privilege;
    int numargs;
    bool hidden;
    int vispriv;
    
    z_servcmdinfo(): fun(NULL), privilege(PRIV_NONE), numargs(0), hidden(false), vispriv(PRIV_NONE) { name[0] = '\0'; }
    z_servcmdinfo(const z_servcmdinfo &n): fun(n.fun), privilege(n.privilege),
        numargs(n.numargs), hidden(n.hidden), vispriv(n.vispriv) { copystring(name, n.name); }
    z_servcmdinfo(const char *_name, z_scmdfun _fun, int _priv, int _numargs, bool _hidden = false, int _vispriv = PRIV_NONE):
        fun(_fun), privilege(_priv), numargs(_numargs), hidden(_hidden), vispriv(_vispriv) { copystring(name, _name); }
    z_servcmdinfo &operator =(const z_servcmdinfo &n)
    {
        if(&n != this)
        {
            copystring(name, n.name);
            fun = n.fun;
            privilege = n.privilege;
            numargs = n.numargs;
            hidden = n.hidden;
            vispriv = n.vispriv;
        }
        return *this;
    }
    ~z_servcmdinfo() {}
    
    bool valid() const { return name[0] && fun; }
    bool cansee(int priv, bool local) const { return valid() && !hidden && ((priv >= privilege && priv >= vispriv) || local); }
    bool canexec(int priv, bool local) const { return valid() && (priv >= privilege || local); }
};

vector<z_servcmdinfo> z_servcommands;
static bool z_initedservcommands = false;
static vector<z_servcmdinfo> *z_servcommandinits = NULL;

static bool addservcmd(const z_servcmdinfo &cmd)
{
    if(!z_initedservcommands)
    {
        if(!z_servcommandinits) z_servcommandinits = new vector<z_servcmdinfo>;
        z_servcommandinits->add(cmd);
        return false;
    }
    z_servcommands.add(cmd);
    return false;
}

static void z_initservcommands()
{
    z_initedservcommands = true;
    
    if(z_servcommandinits)
    {
        loopv(*z_servcommandinits) if((*z_servcommandinits)[i].valid()) addservcmd((*z_servcommandinits)[i]);
        DELETEP(z_servcommandinits);
    }
}

void z_servcmd_set_privilege(const char *cmd, int privilege)
{
    if(!z_initedservcommands) z_initservcommands();
    loopv(z_servcommands) if(!strcmp(z_servcommands[i].name, cmd)) z_servcommands[i].privilege = privilege;
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

#endif // Z_SERVERCOMMANDS_H
