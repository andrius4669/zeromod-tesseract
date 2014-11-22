#ifndef Z_LOADMAP_H
#define Z_LOADMAP_H

SVAR(mappath, "media/map");

bool z_loadmap(const char *mname, stream *&data = mapdata)
{
    string fname;
    if(mappath[0]) formatstring(fname, "%s/%s.ogz", mappath, mname);
    else formatstring(fname, "%s.ogz", mname);
    stream *map = openrawfile(path(fname), "rb");
    if(!map) return false;
    stream::offset len = map->size();
    if(len <= 0 || len > 16<<20) { delete map; return false; }
    DELETEP(data);
    data = map;
    return true;
}

#include "z_servcmd.h"

void z_servcmd_loadmap(int argc, char **argv, int sender)
{
    const char *mname = argc >= 2 ? argv[1] : smapname;
    if(!mname || !mname[0]) { sendf(sender, 1, "ris", N_SERVMSG, "please specify map name"); return; }
    if(z_loadmap(mname)) sendservmsgf("[map \"%s\" loaded]", mname);
    else sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("failed to load map: %s", mname));
}
SCOMMANDNA(loadmap, PRIV_MASTER, z_servcmd_loadmap, 1);

#endif // Z_LOADMAP_H
