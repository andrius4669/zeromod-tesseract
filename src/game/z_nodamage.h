#ifndef Z_NODAMAGE_H
#define Z_NODAMAGE_H

int z_nodamage = 0;
VARF(servernodamage, 0, 0, 2, { if(clients.empty()) z_nodamage = servernodamage; });
static void z_nodamage_trigger(int type)
{
    z_nodamage = servernodamage;
}
Z_TRIGGER(z_nodamage_trigger, Z_TRIGGER_STARTUP);
Z_TRIGGER(z_nodamage_trigger, Z_TRIGGER_NOCLIENTS);

static void z_servcmd_nodamage(int argc, char **argv, int sender)
{
    int val = argc >= 2 && argv[1] && *argv[1] ? atoi(argv[1]) : -1;
    if(val < 0) sendf(sender, 1, "ris", N_SERVMSG,
        tempformatstring("nodamage is %s", z_nodamage <= 0 ? "disabled" : z_nodamage > 1 ? "enabled (without hitpush)" : "enabled"));
    else
    {
        z_nodamage = clamp(val, 0, 2);
        sendservmsgf("nodamage %s", z_nodamage <= 0 ? "disabled" : z_nodamage > 1 ? "enabled (without hitpush)" : "enabled");
    }
}
SCOMMANDNA(nodamage, PRIV_MASTER, z_servcmd_nodamage, 1);

#endif // Z_NODAMAGE_H