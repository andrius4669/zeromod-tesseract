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
SVAR(geoip_color_scheme, "707");

static const struct
{
    uint ip, mask;
    const char *name;
} reservedips[] =
{
    // localhost
    { 0x0000007F, 0x000000FF, "localhost" },            // 127.0.0.0/8
    // lan
    { 0x0000A8C0, 0x0000FFFF, "Local Area Network" },   // 192.168.0.0/16
    { 0x0000000A, 0x000000FF, "Local Area Network" },   // 10.0.0.0/8
    { 0x00004064, 0x0000C0FF, "Local Area Network" },   // 100.64.0.0/10
    { 0x000010AC, 0x0000F0FF, "Local Area Network" },   // 172.16.0.0/12
    { 0x000000C0, 0xF8FFFFFF, "Local Area Network" },   // 192.0.0.0/29
    { 0x000012C6, 0x0000FEFF, "Local Area Network" },   // 198.18.0.0/15
    // autoconfigured lan
    { 0x0000FEA9, 0x0000FFFF, "Local Area Network" }    // 169.254.0.0/16
};

void z_init_geoip()
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
        z_gi = GeoIP_open(geoip_country_database, GEOIP_INDEX_CACHE);
        if(z_gi) GeoIP_set_charset(z_gi, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip country database file \"%s\"", geoip_country_database);
    }
    if(geoip_city_enable && geoip_city_database[0] && !z_gic)
    {
        z_gic = GeoIP_open(geoip_city_database, GEOIP_INDEX_CACHE);
        if(z_gic) GeoIP_set_charset(z_gic, GEOIP_CHARSET_UTF8);
        else logoutf("WARNING: could not open geoip city database file \"%s\"", geoip_city_database);
    }
#else //USE_GEOIP
    if(geoip_country_enable || geoip_city_enable) logoutf("WARNING: GeoIP support was not compiled in");
#endif //USE_GEOIP
}

void z_geoip_resolveclient(clientinfo *ci)
{
    if(!geoip_enable || !ci) return;
    z_init_geoip();
    ci->cleangeoip();
    uint ip = getclientip(ci->clientnum);
    if(!ip) return;
    loopi(sizeof(reservedips)/sizeof(reservedips[0]))
    {
        if((ip & reservedips[i].mask) == reservedips[i].ip)
        {
            ci->geoip_country = newstring(reservedips[i].name);
            break;
        }
    }
    if(ci->geoip_country) return;
    
#ifdef USE_GEOIP
    uchar buf[MAXSTRLEN];
    size_t len;
    if(geoip_show_country && z_gi)
    {
        // const char *country = GeoIP_country_name_by_ipnum(z_gi, ip); // depricated
        GeoIPLookup gl;
        const char *country = GeoIP_country_name_by_ipnum_gl(z_gi, ip, &gl);
        if(country)
        {
            len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)country, strlen(country));
            if(len > 0) { buf[len] = '\0'; ci->geoip_country = newstring((const char *)buf); }
        }
    }
    if((geoip_show_region || geoip_show_city || (geoip_show_country && !ci->geoip_country)) && z_gic)
    {
        GeoIPRecord *gir = GeoIP_record_by_ipnum(z_gic, ip);
        if(gir)
        {
            if(geoip_show_country && !ci->geoip_country)        // country_name is always filled
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
    const char *components[] = { getclienthostname(ci->clientnum), ci->geoip_city, ci->geoip_region, ci->geoip_country };
    int en_components[] = { geoip_show_ip, geoip_show_city, geoip_show_region, geoip_show_country };
    loopi(sizeof(components)/sizeof(components[0])) loopj(2)
    {
        char *s = j ? acstr : ncstr;
        if((j ? en_components[i] : en_components[i] == 1) && components[i])
        {
            if(s[0]) concatstring(s, ", ");
            concatstring(s, components[i]);
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