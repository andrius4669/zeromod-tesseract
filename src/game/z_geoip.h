#ifndef Z_GEOIP_H
#define Z_GEOIP_H 1

#ifdef USE_GEOIP

#include "GeoIP.h"
#include "GeoIPCity.h"

static GeoIP *z_gi = NULL, *z_gic = NULL;
static bool z_geoip_reset_atexit = false;

#endif //USE_GEOIP

static void z_reset_geoip()
{
#ifdef USE_GEOIP
    if(z_gi) { GeoIP_delete(z_gi); z_gi = NULL; }
    if(z_gic) { GeoIP_delete(z_gic); z_gic = NULL; }
#endif
}

VARF(geoip_enable, 0, 0, 1, z_reset_geoip());
VARF(geoip_country_enable, 0, 1, 1, z_reset_geoip());
SVARF(geoip_country_database, "GeoIP.dat", z_reset_geoip());
VARF(geoip_city_enable, 0, 0, 1, z_reset_geoip());
SVARF(geoip_city_database, "GeoLiteCity.dat", z_reset_geoip());

VAR(geoip_show_ip, 0, 2, 2);
VAR(geoip_show_city, 0, 0, 2);
VAR(geoip_show_region, 0, 0, 2);
VAR(geoip_show_country, 0, 1, 2);
VAR(geoip_show_continent, 0, 0, 2);
VAR(geoip_skip_duplicates, 0, 1, 1);
SVAR(geoip_color_scheme, "707");

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
        if(found) z_gi = GeoIP_open(found, GEOIP_INDEX_CACHE);
        if(z_gi) GeoIP_set_charset(z_gi, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip country database file \"%s\"", geoip_country_database);
    }
    if(geoip_city_enable && geoip_city_database[0] && !z_gic)
    {
        const char *found = findfile(geoip_city_database, "rb");
        if(found) z_gic = GeoIP_open(found, GEOIP_INDEX_CACHE);
        if(z_gic) GeoIP_set_charset(z_gic, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip city database file \"%s\"", geoip_city_database);
    }
#else //USE_GEOIP
    if(geoip_country_enable || geoip_city_enable) logoutf("WARNING: GeoIP support was not compiled in");
#endif //USE_GEOIP
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

void z_geoip_show(clientinfo *ci)
{
    if(!geoip_enable || !ci) return;
    char colors[3] = { '7', '7', '7' };
    size_t len = min<size_t>(strlen(geoip_color_scheme), 3);
    for(size_t i = 0; i < len; i++)
    {
        colors[i] = geoip_color_scheme[i];
        if(colors[i] < '0' || colors[i] > '7') colors[i] = '7';
    }
    
    string ncstr, acstr;
    ncstr[0] = acstr[0] = '\0';
    const char *components[] = { getclienthostname(ci->clientnum), ci->geoip_city, ci->geoip_region, ci->geoip_country, ci->geoip_continent };
    bool n_components[] = { geoip_show_ip == 1, geoip_show_city == 1, geoip_show_region == 1, geoip_show_country == 1, geoip_show_continent == 1 };
    bool a_components[] = { geoip_show_ip != 0, geoip_show_city != 0, geoip_show_region != 0, geoip_show_country != 0, geoip_show_continent != 0 };
    loopi(2)
    {
        bool *comp = i ? n_components : a_components;
        char *buf = i ? ncstr : acstr;
        int lastc = -1;
        loopj(sizeof(components)/sizeof(components[0]))
        {
            const char *str = comp[j] ? components[j] : NULL;
            if(str)
            {
                if(geoip_skip_duplicates && lastc >= 0 && !strcmp(str, components[lastc])) continue;
                lastc = j;
                if(buf[0]) concatstring(buf, ", ");
                concatstring(buf, str);
            }
        }
    }

    loopi(2)
    {
        char *s = i ? ncstr : acstr;
        if(s[0])
        {
            defformatstring(buf, "\fs\f%c%s \f%cconnected from \f%c%s\fr", colors[0], colorname(ci), colors[1], colors[2], s);
            packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_SERVMSG);
            sendstring(buf, p);
            ENetPacket *ep = p.finalize();
            if(i) recordpacket(1, ep->data, ep->dataLength);
            loopvj(clients)
            {
                clientinfo *c = clients[j];
                if(c->state.aitype != AI_NONE) continue;
                bool isadmin = c->privilege >= PRIV_ADMIN || c->local;
                if(isadmin == !i) sendpacket(c->clientnum, 1, ep);
            }
        }
    }
}

#endif //Z_GEOIP_H
