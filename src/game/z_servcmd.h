#ifndef Z_SERVCMD_H
#define Z_SERVCMD_H 1

#include "z_servercommands.h"

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

SVAR(servcmd_chars, "");

char *z_servcmd_check(char *text)
{
    const char *c = &servcmd_chars[0];
    while(*c != '\0' && *c != text[0]) c++;
    if(*c == '\0' || text[1] == '\0') return NULL;
    return &text[1];
}

#define Z_MAXSERVCMDARGS 25

void z_servcmd_parse(int sender, char *text)
{
    if(!z_initedservcommands) z_initservcommands();
    
    clientinfo *ci = (clientinfo *)getclientinfo(sender);
    if(!ci) return;
    
    int argc = 1;
    char *argv[Z_MAXSERVCMDARGS];
    argv[0] = text;
    for(size_t i = 1; i < sizeof(argv)/sizeof(argv[0]); i++) argv[i] = NULL;
    char *s = strchr(text, ' ');
    if(s)
    {
        *s++ = '\0';
        while(*s == ' ') s++;
        if(*s) { argv[1] = s; argc++; }
    }
    z_servcmdinfo *cc = NULL;
    loopv(z_servcommands)
    {
        z_servcmdinfo &c = z_servcommands[i];
        if(!c.valid() || strcmp(c.name, text)) continue;
        cc = &c;
        break;
    }
    if(!cc || (cc->hidden && !ci->local && ci->privilege < cc->privilege))
    {
        sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown server command: %s", text));
        return;
    }
    if(!cc->canexec(ci->privilege, ci->local))
    {
        extern const char *privname(int type);
        sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("you need to claim %s to execute this server command", privname(cc->privilege)));
        return;
    }
    int n = cc->numargs ? min(cc->numargs+1, Z_MAXSERVCMDARGS) : Z_MAXSERVCMDARGS;
    if(argv[1]) for(int i = 2; i < n; i++)
    {
        s = strchr(argv[i - 1], ' ');
        if(!s) break;
        *s++ = '\0';
        while(*s == ' ') s++;
        if(*s == '\0') break;
        argv[i] = s;
        argc++;
    }
    cc->fun(argc, argv, sender);
}

#endif //Z_SERVCMD_H
