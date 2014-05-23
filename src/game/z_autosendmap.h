#ifndef Z_AUTOSENDMAP_H
#define Z_AUTOSENDMAP_H 1

#include "z_triggers.h"
#include "z_sendmap.h"

int z_autosendmap = 0, z_connectsendmap = 0;
VARFN(autosendmap, defaultautosendmap, 0, 0, 1, { if(clients.empty()) z_autosendmap = defaultautosendmap; });
VARFN(connectsendmap, defaultconnectsendmap, 0, 0, 2, { if(clients.empty()) z_connectsendmap = defaultconnectsendmap; });
static void z_autosendmap_trigger(int type)
{
    z_autosendmap = defaultautosendmap;
    z_connectsendmap = defaultconnectsendmap;
}
Z_TRIGGER(z_autosendmap_trigger, Z_TRIGGER_STARTUP);
Z_TRIGGER(z_autosendmap_trigger, Z_TRIGGER_NOCLIENTS);

static void z_servcmd_autosendmap(int argc, char **argv, int sender)
{
    int val = argc >= 2 && argv[1] && *argv[1] ? atoi(argv[1]) : -1;
    if(!strcmp(argv[0], "autosendmap"))
    {
        if(val < 0) sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("autosendmap is %s", z_autosendmap ? "enabled" : "disabled"));
        else
        {
            z_autosendmap = clamp(val, 0, 1);
            sendservmsgf("autosendmap %s", z_autosendmap ? "enabled" : "disabled");
        }
    }
    else if(!strcmp(argv[0], "connectsendmap"))
    {
        if(val < 0) sendf(sender, 1, "ris", N_SERVMSG,
            tempformatstring("connectsendmap is %s", z_connectsendmap <= 0 ? "disabled" : z_connectsendmap > 1 ? "enabled for modified maps" : "enabled"));
        else
        {
            z_connectsendmap = clamp(val, 0, 2);
            sendservmsgf("connectsendmap %s",
                z_connectsendmap <= 0 ? "disabled" : z_connectsendmap > 1 ? "enabled for modified maps" : "enabled");
        }
    }
}
SCOMMANDNA(autosendmap, PRIV_MASTER, z_servcmd_autosendmap, 1);
SCOMMANDNA(connectsendmap, PRIV_MASTER, z_servcmd_autosendmap, 1);

#endif //Z_AUTOSENDMAP_H
