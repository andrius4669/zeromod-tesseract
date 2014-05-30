#ifndef Z_MUTES_H
#define Z_MUTES_H 1

#include "z_servcmd.h"
#include "z_triggers.h"

static void z_servcmd_mute(int argc, char **argv, int sender)
{
    if(argc < 2) { sendf(sender, 1, "ris", N_SERVMSG, "please specify client"); return; }
    int val = -1;
    if(argc > 2) val = clamp(atoi(argv[2]), 0, 1);
    int cn;
    clientinfo *ci = NULL;
    int mutetype = 0;

    if(!z_parseclient(argv[1], &cn)) goto cnfail;
    if(cn >= 0)
    {
        ci = getinfo(cn);
        if(!ci || !ci->connected) goto cnfail;
    }

    if(!strcmp(argv[0], "mute")) mutetype = 1;
    else if(!strcmp(argv[0], "unmute")) { mutetype = 1; val = 0; }
    else if(!strcmp(argv[0], "editmute") || !strcmp(argv[0], "emute")) mutetype = 2;
    else if(!strcmp(argv[0], "editunmute") || !strcmp(argv[0], "eunmute")) { mutetype = 2; val = 0; }

    if(mutetype == 1)
    {
        if(ci)
        {
            ci->chatmute = val!=0;
            sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("you %s %s", ci->chatmute ? "muted" : "unmuted", colorname(ci)));
            if(ci->state.aitype == AI_NONE) sendf(ci->clientnum, 1, "ris", N_SERVMSG, tempformatstring("you got %s", ci->chatmute ? "muted" : "unmuted"));
        }
        else loopv(clients)
        {
            ci = clients[i];
            ci->chatmute = val!=0;
            sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("you %s %s", ci->chatmute ? "muted" : "unmuted", colorname(ci)));
            if(ci->state.aitype == AI_NONE) sendf(ci->clientnum, 1, "ris", N_SERVMSG, tempformatstring("you got %s", ci->chatmute ? "muted" : "unmuted"));
        }
    }
    else if(mutetype == 2)
    {
        if(ci)
        {
            if(ci->state.aitype == AI_NONE)
            {
                ci->editmute = val!=0;
                sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("you edit-%s %s", ci->chatmute ? "muted" : "unmuted", colorname(ci)));
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, tempformatstring("you got edit-%s", ci->editmute ? "muted" : "unmuted"));
            }
        }
        else loopv(clients)
        {
            ci = clients[i];
            if(ci->state.aitype != AI_NONE) continue;
            ci->editmute = val!=0;
            sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("you edit-%s %s", ci->editmute ? "muted" : "unmuted", colorname(ci)));
            sendf(ci->clientnum, 1, "ris", N_SERVMSG, tempformatstring("you got edit-%s", ci->editmute ? "muted" : "unmuted"));
        }
    }

    return;
cnfail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[1]));
}
SCOMMANDNA(mute, PRIV_AUTH, z_servcmd_mute, 2);
SCOMMANDNA(unmute, PRIV_AUTH, z_servcmd_mute, 1);
SCOMMANDNA(editmute, PRIV_MASTER, z_servcmd_mute, 2);
SCOMMANDNA(editunmute, PRIV_MASTER, z_servcmd_mute, 1);
SCOMMANDNAH(emute, PRIV_MASTER, z_servcmd_mute, 2);
SCOMMANDNAH(eunmute, PRIV_MASTER, z_servcmd_mute, 1);

bool z_autoeditmute = false;
VARFN(autoeditmute, z_defaultautoeditmute, 0, 0, 1, { if(clients.empty()) z_autoeditmute = z_defaultautoeditmute!=0; });
static void z_trigger_autoeditmute(int type)
{
    z_autoeditmute = z_defaultautoeditmute!=0;
}
Z_TRIGGER(z_trigger_autoeditmute, Z_TRIGGER_STARTUP);
Z_TRIGGER(z_trigger_autoeditmute, Z_TRIGGER_NOCLIENTS);

void z_servcmd_autoeditmute(int argc, char **argv, int sender)
{
    int val = -1;
    if(argc > 1 && argv[1] && *argv[1]) val = atoi(argv[1]);
    if(val >= 0)
    {
        z_autoeditmute = val!=0;
        sendservmsgf("autoeditmute %s", z_autoeditmute ? "enabled" : "disabled");
    }
    else sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("autosendmute is %s", z_autoeditmute ? "enabled" : "disabled"));
}
SCOMMANDNA(autoeditmute, PRIV_MASTER, z_servcmd_autoeditmute, 1);

#endif //Z_MUTES_H