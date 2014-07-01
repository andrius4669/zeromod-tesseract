#ifndef Z_RENAME_H
#define Z_RENAME_H

#include "z_servcmd.h"

static void z_rename(clientinfo *ci, const char *name, bool broadcast = true)
{
    uchar buf[MAXSTRLEN];
    ucharbuf b(buf, MAXSTRLEN);
    putint(b, N_SWITCHNAME);
    sendstring(name, b);
    if(broadcast) ci->messages.put(buf, b.len);
    packetbuf p(MAXSTRLEN, ENET_PACKET_FLAG_RELIABLE);
    putint(p, N_CLIENT);
    putint(p, ci->clientnum);
    putint(p, b.len);
    p.put(buf, b.len);
    sendpacket(ci->ownernum, 1, p.finalize());
}

void z_servcmd_rename(int argc, char **argv, int sender)
{
    clientinfo *ci = NULL;
    int i, cn;
    string namebuf;
    char *name;
    if(argc < 2) { sendf(sender, 1, "ris", N_SERVMSG, "please specify client number"); return; }
    if(!z_parseclient(argv[1], &cn)) goto cnfail;
    if(cn >= 0)
    {
        ci = getinfo(cn);
        if(!ci || !ci->connected) goto cnfail;
    }

    name = ci ? ci->name : namebuf;
    name[0] = '\0';
    if(argc > 2) filtertext(name, argv[2], false, MAXNAMELEN);
    if(!name[0]) copystring(name, "unnamed");

    if(ci) z_rename(ci, name);
    else for(i = 0; i < clients.length(); i++)
    {
        copystring(clients[i]->name, name);
        z_rename(clients[i], name);
    }

    return;

cnfail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[1]));
}
SCOMMANDNA(rename, PRIV_AUTH, z_servcmd_rename, 2);

#endif // Z_RENAME_H
