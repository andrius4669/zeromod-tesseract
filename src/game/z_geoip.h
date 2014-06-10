#ifndef Z_GEOIP_H
#define Z_GEOIP_H

#ifdef USE_GEOIP

#include "GeoIP.h"
#include "GeoIPCity.h"

static GeoIP *z_gi = NULL, *z_gic = NULL;
static bool z_geoip_reset_atexit = false;

#ifndef _WIN32
#define Z_GEOIP_COUNTRY_OPENMODE GEOIP_MMAP_CACHE
#define Z_GEOIP_CITY_OPENMODE GEOIP_MMAP_CACHE
#else
#define Z_GEOIP_COUNTRY_OPENMODE GEOIP_INDEX_CACHE
#define Z_GEOIP_CITY_OPENMODE GEOIP_INDEX_CACHE
#endif

#endif // USE_GEOIP

static void z_reset_geoip_country()
{
#ifdef USE_GEOIP
    if(z_gi) { GeoIP_delete(z_gi); z_gi = NULL; }
#endif
}

static void z_reset_geoip_city()
{
#ifdef USE_GEOIP
    if(z_gic) { GeoIP_delete(z_gic); z_gic = NULL; }
#endif
}

static void z_reset_geoip()
{
    z_reset_geoip_country();
    z_reset_geoip_city();
}

VARF(geoip_enable, 0, 0, 1, z_reset_geoip());
VARF(geoip_country_enable, 0, 1, 1, z_reset_geoip_country());
SVARF(geoip_country_database, "GeoIP.dat", z_reset_geoip_country());
VARF(geoip_city_enable, 0, 0, 1, z_reset_geoip_city());
SVARF(geoip_city_database, "GeoLiteCity.dat", z_reset_geoip_city());
VAR(geoip_show_ip, 0, 2, 2);
VAR(geoip_show_city, 0, 0, 2);
VAR(geoip_show_region, 0, 0, 2);
VAR(geoip_show_country, 0, 1, 2);
VAR(geoip_show_continent, 0, 0, 2);
VAR(geoip_skip_duplicates, 0, 1, 1);
SVAR(geoip_color_scheme, "777");

static const struct
{
    uint ip, mask;
    const char *name;
} reservedips[] =
{
    // localhost
    { 0x7F000000, 0xFF000000, "localhost" },            // 127.0.0.0/8
    // lan
    { 0xC0A80000, 0xFFFF0000, "Local Area Network" },   // 192.168.0.0/16
    { 0x0A000000, 0xFF000000, "Local Area Network" },   // 10.0.0.0/8
    { 0x64400000, 0xFFC00000, "Local Area Network" },   // 100.64.0.0/10
    { 0xAC100000, 0xFFF00000, "Local Area Network" },   // 172.16.0.0/12
    { 0xC0000000, 0xFFFFFFF8, "Local Area Network" },   // 192.0.0.0/29
    { 0xC6120000, 0xFFFE0000, "Local Area Network" },   // 198.18.0.0/15
    // autoconfigured lan
    { 0xA9FE0000, 0xFFFF0000, "Local Area Network" }    // 169.254.0.0/16
};

static void z_init_geoip()
{
    if(!geoip_enable) return;
#ifdef USE_GEOIP
    if(!z_geoip_reset_atexit)
    {
        z_geoip_reset_atexit = true;
        atexit(z_reset_geoip);
    }
    if(geoip_country_enable && geoip_country_database[0] && !z_gi)
    {
        const char *found = findfile(geoip_country_database, "rb");
        if(found) z_gi = GeoIP_open(found, Z_GEOIP_COUNTRY_OPENMODE);
        if(z_gi) GeoIP_set_charset(z_gi, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip country database file \"%s\"", geoip_country_database);
    }
    if(geoip_city_enable && geoip_city_database[0] && !z_gic)
    {
        const char *found = findfile(geoip_city_database, "rb");
        if(found) z_gic = GeoIP_open(found, Z_GEOIP_CITY_OPENMODE);
        if(z_gic) GeoIP_set_charset(z_gic, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip city database file \"%s\"", geoip_city_database);
    }
#undef Z_GEOIP_COUNTRY_OPENMODE
#undef Z_GEOIP_CITY_OPENMODE
#else // USE_GEOIP
    if(geoip_country_enable || geoip_city_enable) logoutf("WARNING: GeoIP support was not compiled in");
#endif // USE_GEOIP
}

#ifdef USE_GEOIP
static const char *z_geoip_decode_continent(const char *cont)
{
    /**/ if(!strcmp(cont, "AF")) return("Africa");
    else if(!strcmp(cont, "AS")) return("Asia");
    else if(!strcmp(cont, "EU")) return("Europe");
    else if(!strcmp(cont, "NA")) return("North America");
    else if(!strcmp(cont, "OC")) return("Oceania");
    else if(!strcmp(cont, "SA")) return("South America");
    else /*********************/ return(NULL);
}
#endif

void z_geoip_resolveclient(clientinfo *ci)
{
    if(!geoip_enable || !ci) return;
    ci->cleangeoip();
    z_init_geoip();
    uint ip = ENET_NET_TO_HOST_32(getclientip(ci->clientnum));
    if(!ip) return;
    bool foundreserved = false;
    loopi(sizeof(reservedips)/sizeof(reservedips[0])) if((ip & reservedips[i].mask) == reservedips[i].ip)
    {
        int    components_v[] = { geoip_show_city, geoip_show_region, geoip_show_country, geoip_show_continent };
        char **components_r[] = { &ci->geoip_city, &ci->geoip_region, &ci->geoip_country, &ci->geoip_continent };
        int bestcomp = -1;
        for(int j = 0; j < 2 && bestcomp < 0; j++)
        {
            loopk(sizeof(components_v)/sizeof(components_v[0])) if(!j ? components_v[k] == 1 : components_v[k])
            {
                bestcomp = k;
                break;
            }
        }
        *components_r[max(bestcomp, 0)] = newstring(reservedips[i].name);
        foundreserved = true;
        break;
    }
    if(foundreserved) return;

#ifdef USE_GEOIP
    uchar buf[MAXSTRLEN];
    size_t len;
    if(z_gi && (geoip_show_continent || geoip_show_country))
    {
        int country_id = GeoIP_id_by_ipnum(z_gi, ip);
        if(geoip_show_continent)
        {
            const char *shortcont = GeoIP_continent_by_id(country_id);
            const char *continent = shortcont ? z_geoip_decode_continent(shortcont) : NULL;
            if(continent) ci->geoip_continent = newstring(continent);
        }
        if(geoip_show_country)
        {
            const char *country = GeoIP_country_name_by_id(z_gi, country_id);
            if(country)
            {
                len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)country, strlen(country));
                if(len > 0) { buf[len] = '\0'; ci->geoip_country = newstring((const char *)buf); }
            }
        }
    }
    if(z_gic && (geoip_show_city || geoip_show_region || (geoip_show_country && !ci->geoip_country) ||
        (geoip_show_continent && !ci->geoip_continent)))
    {
        GeoIPRecord *gir = GeoIP_record_by_ipnum(z_gic, ip);
        if(gir)
        {
            if(geoip_show_continent && !ci->geoip_continent)
            {
                const char *continent = z_geoip_decode_continent(gir->continent_code);
                if(continent) ci->geoip_continent = newstring(continent);
            }
            if(geoip_show_country && !ci->geoip_country && gir->country_name)
            {
                len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)gir->country_name, strlen(gir->country_name));
                if(len > 0) { buf[len] = '\0'; ci->geoip_country = newstring((const char *)buf); }
            }
            if(geoip_show_region && gir->region)
            {
                const char *region = GeoIP_region_name_by_code(gir->country_code, gir->region);
                if(region)
                {
                    len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)region, strlen(region));
                    if(len > 0) { buf[len] = '\0'; ci->geoip_region = newstring((const char *)buf); }
                }
            }
            if(geoip_show_city && gir->city)
            {
                len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)gir->city, strlen(gir->city));
                if(len > 0) { buf[len] = '\0'; ci->geoip_city = newstring((const char *)buf); }
            }
            GeoIPRecord_delete(gir);
        }
    }
#endif //USE_GEOIP
}

static void z_geoip_gencolors(char *cbuf)
{
    size_t slen = strlen(geoip_color_scheme), len = min<size_t>(slen, 3);
    for(size_t i = 0; i < len; i++)
    {
        cbuf[i] = geoip_color_scheme[i];
        if(cbuf[i] < '0' || cbuf[i] > '7') cbuf[i] = '7';
    }
    for(size_t i = len; i < 3; i++) cbuf[i] = '7';
}

static void z_geoip_print(vector<char> &buf, clientinfo *ci, bool admin)
{
    buf.setsize(0);
    const char *comp[] =
    {
        (admin ? geoip_show_ip : geoip_show_ip == 1) ? getclienthostname(ci->clientnum) : NULL,
        (admin ? geoip_show_city : geoip_show_city == 1) ? ci->geoip_city : NULL,
        (admin ? geoip_show_region : geoip_show_region == 1) ? ci->geoip_region : NULL,
        (admin ? geoip_show_country : geoip_show_country == 1) ? ci->geoip_country : NULL,
        (admin ? geoip_show_continent : geoip_show_continent == 1) ? ci->geoip_continent : NULL
    };
    int lastc = -1;
    loopi(sizeof(comp)/sizeof(comp[0])) if(comp[i])
    {
        if(geoip_skip_duplicates && lastc >= 0 && !strcmp(comp[i], comp[lastc])) continue;
        lastc = i;
        if(buf.length()) { buf.add(','); buf.add(' '); }
        buf.put(comp[i], strlen(comp[i]));
    }
    buf.add('\0');
}

void z_geoip_show(clientinfo *ci)
{
    if(!geoip_enable || !ci) return;

    char colors[3];
    z_geoip_gencolors(colors);

    vector<char> cbufs[2];
    packetbuf *qpacks[2] = { NULL, NULL };
    for(int i = demorecord ? -1 : 0; i < clients.length(); i++) if(i < 0 || clients[i]->state.aitype==AI_NONE)
    {
        bool isadmin = i >= 0 && (clients[i]->privilege >= PRIV_ADMIN || clients[i]->local);
        int idx = isadmin ? 1 : 0;
        if(!qpacks[idx])
        {
            if(cbufs[idx].empty()) z_geoip_print(cbufs[idx], ci, isadmin);
            if(cbufs[idx].length() <= 1) continue;
            qpacks[idx] = new packetbuf(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            packetbuf &p = *qpacks[idx];
            putint(p, N_SERVMSG);
            sendstring(tempformatstring("\f%c%s \f%cconnected from \f%c%s", colors[0], colorname(ci), colors[1], colors[2], cbufs[idx].getbuf()), p);
            p.finalize();
        }
        if(i >= 0) sendpacket(clients[i]->clientnum, 1, qpacks[idx]->packet);
        else recordpacket(1, qpacks[idx]->packet->data, qpacks[idx]->packet->dataLength);
    }
    loopi(2) DELETEP(qpacks[i]);
}

void z_servcmd_geoip(int argc, char **argv, int sender)
{
    int i, cn;
    clientinfo *ci, *senderci = getinfo(sender);
    vector<clientinfo *> cis;
    vector<char> buf;
    char c[3];
    bool isadmin = senderci->privilege>=PRIV_ADMIN || senderci->local;
    for(i = 1; i < argc; i++)
    {
        if(!z_parseclient(argv[i], &cn)) goto fail;
        if(cn < 0)
        {
            cis.shrink(0);
            loopvj(clients) if(clients[j]->state.aitype==AI_NONE && (!clients[j]->spy || isadmin)) cis.add(clients[j]);
            break;
        }
        ci = getinfo(cn);
        if(!ci || !ci->connected || (ci->spy && !isadmin)) goto fail;
        if(cis.find(ci)<0) cis.add(ci);
    }

    if(cis.empty()) { sendf(sender, 1, "ris", N_SERVMSG, "please specify client number"); return; }

    z_geoip_gencolors(c);

    for(i = 0; i < cis.length(); i++)
    {
        if(cis[i]->state.aitype==AI_NONE) z_geoip_print(buf, cis[i], isadmin);
        else buf.setsize(0);
        if(buf.length() > 1) sendf(sender, 1, "ris", N_SERVMSG,
            tempformatstring("\f%c%s \f%cis connected from \f%c%s", c[0], colorname(cis[i]), c[1], c[2], buf.getbuf()));
        else sendf(sender, 1, "ris", N_SERVMSG,
            tempformatstring("\f%cfailed to get any geoip information about \f%c%s", c[1], c[0], colorname(cis[i])));
    }
    return;
fail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[i]));
}
SCOMMANDN(geoip, PRIV_NONE, z_servcmd_geoip);
SCOMMANDNH(getip, PRIV_NONE, z_servcmd_geoip);

#endif // Z_GEOIP_H
