#include <iostream>
#include <cstdio>
#include <fstream>
#include <istream>
#include <vector>
#include <regex>
#include <string>
#include <iterator>

using namespace std;

typedef vector<string> vs;
typedef string ss;

struct events_block
{
    ss start;
    ss tab;
    vs workers;
    vs multi;
    vs mutex;
    vs custom;
    ss end;
};
struct http_block
{
    ss start;
    ss tab;
    vs include;
    vs global_headers;
    vs log_formatting;
    vs limit_concurrency;
    vs fast_cgi;
    vs load_balancer;
    vs geoip;
    vs geoip_blocks_allow;
    vs geoip_blocks_deny;
    vs custom;
    ss end;
};
struct server_block
{
    ss start;
    ss tab;
    vs listen;
    vs allow;
    vs deny;
    ss server_name;
    ss root_folder;
    vs global_logs_status;
    vs global_logs_settings;
    vs ssl;
    vs buffers;
    vs timeouts;
    vs keepalive;
    ss server_token;
    vs gzip;
    vs caching;
    vs custom;
    ss end;
};
struct location_block
{
    ss start;
    ss tab;
    ss location_name;
    vs auth;
    vs fast_cgi;
    ss expires;
    vs custom_logs_status;
    vs custom_logs_settings;
    vs custom_headers;
    ss proxy_pass;
    vs custom;
    ss end;
};
struct nginx_conf
{
    events_block events;
    http_block http;
    server_block server;
    vector<location_block> locations;
    ss location;
};

nginx_conf config;

vs readInputValues()
{
    vs values;
    string input;

    getline(cin, input);

    istringstream iss(input);
    string value;
    while (iss >> value)
    {
        values.push_back(value);
    }

    return values;
}

void setEventsBlock()
{
    ss workers, multi, mutex, custom;
    int cnt(1);

    config.events.start = "\n\tevents {";
    config.events.tab = "\n\t\t";
    config.events.end = "\n\t}\n";

    cout << "\n\t\n\t\t\t[Конфигурация блока events]\n";
    cout << "\n\t! Если хотите пропустить настройку, оставьте поле пустым и нажмите Enter";
    cout << "\n\t! Если хотите установить настройку по умолчанию, Укажите default\n";

    cout << "\n\tУкажите количество одновременных соединений для каждого процесса: ";
    getline(cin, workers);
    if (workers != "")
        if (workers != "default")
            config.events.workers.push_back("worker_connections\t" + workers + ";");
        else
            config.events.workers.push_back("worker_connections 1024;");

    cout << "\n\tУкажите, должен ли рабочий процесс принимать несколько соединений одновременно [yes/no]: ";
    getline(cin, multi);
    if (multi != "")
    {
        if (multi == "default")
            config.events.multi.push_back("multi_accept on;");
        if (multi == "yes")
            config.events.multi.push_back("multi_accept on;");
        if (multi == "no")
            config.events.multi.push_back("multi_accept off;");
    }

    cout << "\n\tУкажите, должен ли рабочий процесс использовать мьютекс при приеме новых соединений [yes/no]: ";
    getline(cin, mutex);
    if (mutex != "")
    {
        if (mutex == "default")
            config.events.mutex.push_back("accept_mutex on;");
        if (mutex == "yes")
            config.events.mutex.push_back("accept_mutex on;");
        if (mutex == "no")
            config.events.mutex.push_back("accept_mutex off;");
    }

    cout << "\n\tУкажите дополнительные настройки для блока events [no чтобы закончить ввод]: ";
    cout << "\n\t\tДоп. настройка " << cnt << ": ";
    getline(cin, custom);
    while (custom != "no")
    {
        config.events.custom.push_back(custom + (custom.back() == ';' ? "" : ";"));
        cnt++;
        cout << "\n\t\tДоп. настройка " << cnt << ": ";
        getline(cin, custom);
    }

    cout << "\n\t\t\t\t[Конец конфигурации блока events]\n";
}

void setHttpBlock()
{
    config.http.start = "\n\thttp {";
    config.http.tab = "\n\t\t";
    config.http.end = "\n\t}";
    config.http.include.push_back("include mime.types;");
    config.http.include.push_back("default_type application/octet-stream;");

    vs servers, regions, selected_regions;
    ss header_st, gl_header_name, gl_header_data,
        log_st, log_format_name, log_format_data,
        limc_st, lim_conn_param, lim_conn_name, lim_conn_size,
        fcgi_st, fcgi_path, fcgi_custom,
        ups_st, ups_name, ups_type, ups_server,
        geo_st, geo_path, geobl_regs,
        custom;
    ss upstream_time_format = R"(log_format upstream_time '$remote_addr - $remote_user [$time_local] '
                                '"$request" $status $body_bytes_sent '
                                '"$http_referer" "$http_user_agent"'
                                'rt=$request_time uct="$upstream_connect_time" '
                                'uht="$upstream_header_time" urt="$upstream_response_time"';
)";
    ss main_format = R"(log_format  main '$remote_addr - $remote_user [$time_local] "$request" '
                        '$status $body_bytes_sent "$http_referer" '
                        '"$http_user_agent" "$http_x_forwarded_for"';
)";
    regions = {
        "default no;",
        "~^RU:  yes;",
        "~^BY:  yes;",
        "~^AM:  yes;",
        "~^KZ:  yes;",
        "~^KG:  yes;",
        "UA:40  yes;",
        "UA:43  yes;",
        "UA:14  yes;",
        "UA:09  yes;",
        "UA:23  yes;",
        "UA:65  yes;",
        "GE:AB  yes;"};

    int cnt(1);

    cout << "\n\t\n\t\t\t[Конфигурация блока http]\n";
    cout << "\n\t! Если хотите пропустить настройку, оставьте поле пустым и нажмите Enter";
    cout << "\n\t! Если хотите установить настройку по умолчанию, Укажите default\n";

    cout << "\n\tУкажите настройку глобальных хэдеров [default/custom]: ";
    getline(cin, gl_header_name);

    if (header_st != "no")
    {
        if (header_st == "default")
        {
            config.http.global_headers.push_back(R"(add_header Access-Control-Allow-Origin;)");
            config.http.global_headers.push_back(R"(add_header Caching $upstream_cache_status;)");
            config.http.global_headers.push_back(R"(add_header Cache-Control public;)");
            config.http.global_headers.push_back(R"(add_header Vary Accept-Encoding;)");
            config.http.global_headers.push_back(R"(add_header X-Content-Type-Options nosniff;)");
            config.http.global_headers.push_back(R"(add_header X-XSS-Protection "1; mode=block";)");
        }

        if (header_st == "custom")
        {
            cout << "\n\tУкажите хэдер " << cnt << ": ";
            cout << "\n\t\tИмя: ";
            getline(cin, gl_header_name);
            while (gl_header_name != "no")
            {
                cout << "\n\t\tДанные: ";
                getline(cin, gl_header_data);
                config.http.global_headers.push_back(
                    "add_header " + gl_header_name + " " + gl_header_data + ";");
                cnt++;
                cout << "\n\tУкажите хэдер " << cnt << ": ";
                cout << "\n\t\tИмя: ";
                getline(cin, gl_header_name);
            }
        }
    }

    cout << "\n\tУкажите используемые форматы логов [default/custom]: ";
    getline(cin, log_st);
    if (log_st != "")
    {
        if (log_st == "default")
        {
            config.http.log_formatting.push_back(upstream_time_format);
            config.http.log_formatting.push_back(main_format);
            cout << "\n\tВ файл конфигурации были добавлены следующие виды форматирования:\n";
            cout << upstream_time_format << endl;
            cout << main_format << endl;
        }

        if (log_st == "custom")
        {
            cnt = 1;
            config.http.log_formatting.push_back(upstream_time_format);
            config.http.log_formatting.push_back(main_format);
            cout << "\n\tВ файл конфигурации были добавлены следующие виды форматирования:\n";
            cout << upstream_time_format << endl;
            cout << main_format << endl;

            cout << "\n\tНовый формат " << cnt << ": ";
            cout << "\n\t\tИмя: ";
            getline(cin, log_format_name);

            while (gl_header_name != "no")
            {
                cout << "\n\t\tДанные: ";
                getline(cin, log_format_data);

                config.http.log_formatting.push_back("log_format " + log_format_name + " " + log_format_data + ";");
                cnt++;

                cout << "\n\tНовый формат" << cnt << ": ";
                cout << "\n\t\tИмя: ";
                getline(cin, gl_header_name);
            }
        }
    }

    cout << "\n\tУкажите зоны ограничения соединений [default/custom]: ";
    getline(cin, limc_st);
    if (limc_st != "")
    {
        if (limc_st == "default")
        {
            config.http.limit_concurrency.push_back(
                "limit_conn_zone $binary_remote_addr zone=per_ip:5m;");
            cout << "\n\tВ файл конфигурации добавлена зона ограничения соединений с именем per_ip и размером 5 мегабайт.";
            cout << "\n\t\tДанная зона ставит ограничения на количество соединений для каждого IP-адреса по-отдельности\n";
        }
        if (limc_st == "custom")
        {
            cnt = 1;
            cout << "\n\tУкажите зону ограничения соединений " << cnt << " [no чтобы закончить ввод]: ";
            cout << "\n\t\tИмя: ";
            getline(cin, lim_conn_name);
            cout << "\n\t\tПеременная ограничения: ";
            getline(cin, lim_conn_param);
            cout << "\n\t\tРазмер зоны: ";
            getline(cin, lim_conn_size);
            while (gl_header_name != "no")
            {
                config.http.limit_concurrency.push_back(
                    "limit_conn_zone" + lim_conn_param + " zone=" + lim_conn_name + ":" + lim_conn_size + ";");
                cnt++;
                cout << "\n\tУкажите зону ограничения соединений " << cnt << " [no чтобы закончить ввод]: ";
                cout << "\n\t\tИмя: ";
                getline(cin, lim_conn_name);
                cout << "\n\t\tПеременная ограничения: ";
                getline(cin, lim_conn_param);
                cout << "\n\t\tРазмер зоны: ";
                getline(cin, lim_conn_size);
            }
        }
    }

    cout << "\n\tИспользовать модуль FastCGI? [yes/no]: ";
    getline(cin, fcgi_st);
    if (fcgi_st == "yes")
    {
        config.http.include.push_back("include fastcgi_params;");
        config.http.include.push_back("include fastcgi.conf;");

        cout << "\n\t\tУкажите тип настроек FastCGI [default/custom]: ";
        getline(cin, fcgi_st);

        if (fcgi_st == "default")
        {
            cout << "\n\t\t\tКакой путь использовать для кэша FastCGI? [default/custom]: ";
            getline(cin, fcgi_path);
            if (fcgi_path == "default" || fcgi_path == "")
            {
                config.http.fast_cgi.push_back(
                    "fastcgi_cache_path " +
                    config.location + "/cache levels=1:2 keys_zone=microcache:10m max_size=500m inactive=10m;");
                config.http.fast_cgi.push_back(
                    R"(fastcgi_cache_key "$scheme$request_method$host$request_uri";)");
                config.http.fast_cgi.push_back(
                    "fastcgi_ignore_headers Cache-Control Expires Set-Cookie ;");
            }
            else
            {
                cout << "\n\t\t\t\tУкажите кастомный путь: ";
                getline(cin, fcgi_path);
                config.http.fast_cgi.push_back(
                    "fastcgi_cache_path " +
                    fcgi_path + "/cache levels=1:2 keys_zone=microcache:10m max_size=500m inactive=10m;");
                config.http.fast_cgi.push_back(
                    R"(fastcgi_cache_key "$scheme$request_method$host$request_uri";)");
                config.http.fast_cgi.push_back(
                    "fastcgi_ignore_headers Cache-Control Expires Set-Cookie ;");
            }
        }

        if (fcgi_st == "custom")
        {
            cnt = 1;
            cout << "\n\tУкажите настройку FastCGI " << cnt << " [no чтобы закончить ввод]: ";
            getline(cin, fcgi_custom);
            while (fcgi_custom != "no")
            {
                config.http.fast_cgi.push_back(
                    fcgi_custom + (fcgi_custom.back() == ';' ? "" : ";"));
                cnt++;
                cout << "\n\tУкажите настройку FastCGI " << cnt << " [no чтобы закончить ввод]: ";
                getline(cin, fcgi_custom);
            }
        }
    }

    cout << "\n\tСоздать группу серверов для балансировщика нагрузки? [yes/no]: ";
    getline(cin, ups_st);
    if (ups_st == "yes")
    {
        cout << "\n\t\tУкажите имя группы серверов: ";
        getline(cin, ups_name);
        cout << "\n\t\tУкажите алгоритм балансировки: ";
        getline(cin, ups_type);
        cout << "\n\t\tУкажите сервера группы (ip:port) [no чтобы закончить ввод]: ";

        cnt = 1;
        cout << "\n\t\t\tServer " << cnt << ": ";
        getline(cin, ups_server);
        while (ups_server != "no" && ups_server != "")
        {
            cnt++;
            servers.push_back(ups_server);
            cout << "\n\t\t\tServer " << cnt << ": ";
            getline(cin, ups_server);
        }

        config.http.load_balancer.push_back(
            "upstream " + ups_name + " {");
        config.http.load_balancer.push_back(
            "\t" + ups_type + ";");
        for (auto server : servers)
            config.http.load_balancer.push_back(
                "\tserver " + server + ";");
        config.http.load_balancer.push_back(
            "}");
    }

    cout << "\n\tИспользовать модуль Geoip2? [yes/no]: ";
    getline(cin, geo_st);
    if (geo_st == "yes")
    {

        cout << "\n\tКакой путь использовать для доступа к GeoLite2-City.mmdb? [default/custom]: ";
        getline(cin, geo_path);
        if (geo_path != "")
        {
            if (geo_path == "default")
                config.http.geoip.push_back(
                    "geoip2 /usr/share/GeoIP/GeoLite2-City.mmdb {");
            if (geo_path != "default")
            {
                cout << "\n\tВведите путь до GeoLite2-City.mmdb: ";
                getline(cin, geo_path);
                config.http.geoip.push_back(
                    "geoip2 " + geo_path + (geo_path.back() == ';' ? "" : ";") + "GeoLite2-City.mmdb {");
            }
            config.http.geoip.push_back("\tauto_reload 60m;");
            config.http.geoip.push_back("\t$geoip2_metadata_city_build metadata build_epoch;");
            config.http.geoip.push_back("\t$geoip2_data_country_name country names en;");
            config.http.geoip.push_back("\t$geoip2_data_country_code country iso_code;");
            config.http.geoip.push_back("\t$geoip2_data_city_name city names en;");
            config.http.geoip.push_back("\t$geoip2_data_region_name subdivisions 0 names en;");
            config.http.geoip.push_back("\t$geoip2_data_state_code  subdivisions 0 iso_code;");
            config.http.geoip.push_back("}");
        }

        cout << "\n\tТекущие разрешенные регионы: ";
        cout << "\n\t\t[1]  RU --> Россия";
        cout << "\n\t\t[2]  BY --> Белоруссия";
        cout << "\n\t\t[3]  AM --> Армения";
        cout << "\n\t\t[4]  KZ --> Казахстан";
        cout << "\n\t\t[5]  KG --> Кыргызстан";
        cout << "\n\t\t[6]  UA:4 --> Севастополь";
        cout << "\n\t\t[7]  UA:4 --> Крым";
        cout << "\n\t\t[8]  UA:1 --> Донецкая область";
        cout << "\n\t\t[9]  UA:0 --> Луганская область";
        cout << "\n\t\t[10] UA:2 --> Запорожская область";
        cout << "\n\t\t[11] UA:6 --> Херсонская область";
        cout << "\n\t\t[12] GE:A --> Абхазия";

        cout << "\n\tУкажите id регионoв, которые будут иметь доступ при внештатных ситуциях [no чтобы закончить ввод]: ";
        getline(cin, geobl_regs);

        string delimiter = " ";
        size_t pos = 0;
        string token;

        while ((pos = geobl_regs.find(delimiter)) != string::npos)
        {
            token = geobl_regs.substr(0, pos);
            int index = stoi(token);

            if (index >= 0 && index < regions.size())
                selected_regions.push_back(regions[index]);

            geobl_regs.erase(0, pos + delimiter.length());
        }

        if (!geobl_regs.empty())
        {
            int index = stoi(geobl_regs);
            if (index >= 0 && index < regions.size())
                selected_regions.push_back(regions[index]);
        }

        config.http.geoip_blocks_allow.push_back(
            R"(map "$geoip2_data_country_code:$geoip2_data_state_code" $allowed_reg {)");
        config.http.geoip_blocks_allow.push_back(
            "\tdefault no;");
        for (auto reg : regions)
            config.http.geoip_blocks_allow.push_back(
                "\t" + reg);
        config.http.geoip_blocks_allow.push_back(
            "}");

        config.http.geoip_blocks_deny.push_back(
            R"(map "$geoip2_data_country_code:$geoip2_data_state_code" $allowed_reg {)");
        config.http.geoip_blocks_deny.push_back(
            "\tdefault no;");
        for (auto reg : selected_regions)
            config.http.geoip_blocks_deny.push_back(
                "\t" + reg);
        config.http.geoip_blocks_deny.push_back(
            "}");
    }

    cnt = 1;
    cout << "\n\tУкажите дополнительные настройки для блока http [no чтобы закончить ввод]: ";
    cout << "\n\t\tДоп. настройка " << cnt << ": ";
    getline(cin, custom);
    while (custom != "no")
    {
        config.http.custom.push_back(custom + (custom.back() == ';' ? "" : ";"));
        cnt++;
        cout << "\n\t\tДоп. настройка " << cnt << ": ";
        getline(cin, custom);
    }

    cout << "\n\t\t\t\t[Конец конфигурации блока http]\n";
}

void setServerBlock()
{
    config.server.start = "\n\t\tserver {";
    config.server.tab = "\n\t\t\t";
    config.server.end = "\n\t\t}";

    int cnt(1);
    vs listening_ports, allow_hosts, deny_hosts, gzip_files, gzip_disable;
    ss custom, server_name, root_dir,
        alog_st, elog_st, alog_path, elog_path, alog_type, elog_type,
        ssl_st, ssl_crt, ssl_key, ssl_listen,
        buf_st, tmout_st, kpalive_st, ver_st,
        gzip_st, cache_st;

    cout << "\n\t\n\t\t\t[Конфигурация блока server]\n";
    cout << "\n\t! Если хотите пропустить настройку, оставьте поле пустым и нажмите Enter";
    cout << "\n\t! Если хотите установить настройку по умолчанию, Укажите default\n";

    cout << "\n\tУкажите прослушиваемые порты [через пробел]: ";
    listening_ports = readInputValues();
    for (auto port : listening_ports)
        config.server.listen.push_back("listen " + port + ";");

    cout << "\n\tУкажите разрешенные хосты [через пробел]: ";
    allow_hosts = readInputValues();
    for (auto host : allow_hosts)
        config.server.allow.push_back("allow " + host + ";");

    cout << "\n\tУкажите запрещенные хосты [через пробел]: ";
    deny_hosts = readInputValues();
    for (auto host : deny_hosts)
        config.server.deny.push_back("deny " + host + ";");

    cout << "\n\tУкажите имя сервера [default]: ";
    getline(cin, server_name);
    if (server_name == "default")
        cout << "\n\tИмя сервера установлено как: localhost";
    config.server.server_name = config.server.tab + "server_name ";
    config.server.server_name += server_name == "" ? "localhost" : server_name;

    cout << "\n\tУкажите корневой каталог сайта [default]: ";
    if (root_dir == "default")
        cout << "\n\tКорневой каталог сайта установлен как: /etc/nginx/site/";
    getline(cin, root_dir);
    config.server.root_folder = config.server.tab + "root ";
    config.server.root_folder += root_dir == "" ? "/etc/nginx/site/" : root_dir;

    cout << "\n\tВключить логирование ошибок для всех директорий? [yes/no]: ";
    getline(cin, elog_st);
    if (elog_st != "")
    {
        if (elog_st == "yes")
        {
            config.server.global_logs_status.push_back("error_log on;");
            cout << "\n\t\tУкажите каталог для хранения логов ошибок [default]: ";
            getline(cin, elog_path);
            elog_path = elog_path == "default" ? "/etc/nginx/logs/" : elog_path;

            cout << "\n\t\tУкажите формат логов ошибок [default]: ";
            getline(cin, elog_type);
            elog_type = elog_type == "default" ? "main" : elog_type;
            config.server.global_logs_settings.push_back(
                "error_log " + elog_path + (elog_path.back() == '/' ? "" : "/") +
                "error_base.log " + elog_type + ";");
        }
        if (elog_st == "no")
            config.server.global_logs_status.push_back("error_log off;");
    }

    cout << "\n\tВключить логирование доступа для всех директорий? [yes/no]: ";
    getline(cin, alog_st);
    if (alog_st != "")
    {
        if (alog_st == "yes")
        {
            config.server.global_logs_status.push_back("access_log on;");

            cout << "\n\t\tУкажите каталог для хранения логов ошибок [default]: ";
            getline(cin, alog_path);
            alog_path = alog_path == "default" ? "/etc/nginx/logs/" : alog_path;

            cout << "\n\t\tУкажите формат логов ошибок [default]: ";
            getline(cin, alog_type);
            alog_type = alog_type == "default" ? "main" : alog_type;

            config.server.global_logs_settings.push_back(
                "access_log " + alog_path + (alog_path.back() == '/' ? "" : "/") +
                "access_base.log " + alog_type + ";");
        }
        if (alog_st == "no")
            config.server.global_logs_status.push_back("access_log off;");
    }

    cout << "\n\tВключить шифрование SSL/TLS? [yes/no]: ";
    getline(cin, ssl_st);
    if (ssl_st == "yes")
    {
        cout << "\n\t\tУкажите прослушивающий порт: ";
        getline(cin, ssl_listen);
        cout << "\n\t\tУкажите файл ssl-сертификата (.crt): ";
        getline(cin, ssl_crt);
        cout << "\n\t\tУкажите файл ssl-ключа (.key): ";
        getline(cin, ssl_key);

        config.server.listen.push_back("listen " + ssl_listen + " ssl;");
        config.server.ssl.push_back("ssl_certificate " + ssl_crt + ";");
        config.server.ssl.push_back("ssl_certificate_key " + ssl_key + ";");
        config.server.ssl.push_back("ssl_session_cache shared:SSL:1m;");
        config.server.ssl.push_back("ssl_session_timeout 5m;");
        config.server.ssl.push_back("ssl_ciphers HIGH:!aNULL:!MD5;");
        config.server.ssl.push_back("ssl_prefer_server_ciphers on;");
    }

    cout << "\n\tВключить ограничения буферизации сообщений? [yes/no]: ";
    getline(cin, buf_st);
    if (ssl_st == "yes")
    {
        config.server.buffers.push_back("client_body_buffer_size 16k;");
        config.server.buffers.push_back("client_header_buffer_size 1k;");
        config.server.buffers.push_back("client_max_body_size 8m;");
        config.server.buffers.push_back("large_client_header_buffers 2 1k;");
    }

    cout << "\n\tВключить таймаут ожидания для передачи запроса клиента? [yes/no]: ";
    getline(cin, tmout_st);
    if (ssl_st == "yes")
    {
        cout << "\n\t\tУкажите таймаут для хэдера запроса: ";
        getline(cin, tmout_st);
        config.server.timeouts.push_back("client_header_timeout " + tmout_st + ";");
        cout << "\n\t\tУкажите таймаут для тела запроса: ";
        getline(cin, tmout_st);
        config.server.timeouts.push_back("client_body_timeout " + tmout_st + ";");
    }

    cout << "\n\tВключить таймаут ожидания для активного соединения? [yes/no]: ";
    getline(cin, kpalive_st);
    if (kpalive_st == "yes")
        config.server.keepalive.push_back("keepalive_timeout 65;");

    cout << "\n\tВключить таймаут ожидания для отправки данных клиенту? [yes/no]: ";
    getline(cin, kpalive_st);
    if (kpalive_st == "yes")
        config.server.keepalive.push_back("send_timeout 10;");

    cout << "\n\tОтключить отображение информации о версии Nginx в хэдерах? [yes/no]: ";
    getline(cin, ver_st);
    if (ver_st == "yes")
        config.server.server_token = config.server.tab + "server_tokens off;";

    cout << "\n\tВключить сжатие ответов сервера с помощью gzip? [yes/no]: ";
    getline(cin, gzip_st);
    if (gzip_st == "yes")
    {
        config.server.gzip.push_back("gzip on;");

        cout << "\n\t\tУкажите минимальный размер сжимаемых файлов: ";
        getline(cin, gzip_st);
        config.server.gzip.push_back("gzip_min_length " + gzip_st + ";");

        cout << "\n\t\tУкажите степень сжатия файлов: ";
        getline(cin, gzip_st);
        config.server.gzip.push_back("gzip_comp_level " + gzip_st + ";");

        cout << "\n\t\tУкажите типы сжимаемых файлов через пробел(Пр.: css plain): ";
        gzip_files = readInputValues();
        for (auto file : gzip_files)
            config.server.gzip.push_back("gzip_types text/" + file + ";");

        config.server.gzip.push_back(R"(gzip_disable "msie6";)");
    }

    cout << "\n\tОграничитить кэширование FastCGI по определенным правилам? [yes/no]: ";
    getline(cin, cache_st);
    if (cache_st == "yes")
    {
        config.server.caching.push_back("set $no_cache 0;");
        cout << "\n\tУкажите правила ограничений [no чтобы закончить ввод]: ";
        cnt = 1;
        cout << "\n\t\tПравило " << cnt << ": ";
        getline(cin, custom);

        while (custom != "no")
        {
            config.server.caching.push_back("if (" + custom + ") { set $no_cache 1; }");
            cnt++;
            cout << "\n\t\tПравило " << cnt << ": ";
            getline(cin, custom);
        }
    }

    if (config.http.geoip.size() != 0)
    {
        config.server.custom.push_back("if ($allowed_reg = no) {");
        config.server.custom.push_back("\treturn 444;");
        config.server.custom.push_back("}");
    }

    cnt = 1;
    cout << "\n\tУкажите дополнительные настройки для блока server [no чтобы закончить ввод]: ";
    cout << "\n\t\tДоп. настройка " << cnt << ": ";
    getline(cin, custom);
    while (custom != "no")
    {
        config.server.custom.push_back(custom + (custom.back() == ';' ? "" : ";"));
        cnt++;
        cout << "\n\t\tДоп. настройка " << cnt << ": ";
        getline(cin, custom);
    }

    cout << "\n\t\t\t\t[Конец конфигурации блока server]\n";
}

void setLocations()
{

    int cnt(1);

    ss loc_st, exp_st, prx_st, prx_name, bal_st,
        auth_st, auth_path, auth_log, cgi_st,
        celog_st, celog_path, celog_type, celog_name,
        prxhc_st, prxhs_st, custom, hd_name, hd_status;

    location_block default_loc;

    cout << "\n\t\n\t\t\t[Конфигурация блоков location]\n";
    cout << "\n\t! Если хотите пропустить настройку, оставьте поле пустым и нажмите Enter";
    cout << "\n\t! Если хотите установить настройку по умолчанию, Укажите default\n";

    default_loc.location_name = R"( ~ /\.ht )";
    default_loc.start = "\n\t\t\tlocation " + default_loc.location_name + "{";
    default_loc.tab = "\n\t\t\t\t";
    default_loc.end = "\n\t\t\t}\n";
    default_loc.custom.push_back("deny  all;");

    config.locations.push_back(default_loc);

    cout << "\n\tСоздать новую локацию? [yes/no]: ";
    getline(cin, loc_st);
    while (loc_st == "yes")
    {
        location_block nloc;

        cout << "\n\t\tУкажите имя локации: ";
        getline(cin, nloc.location_name);

        nloc.start = "\n\t\t\tlocation " + nloc.location_name + " {";
        nloc.tab = "\n\t\t\t\t";
        nloc.end = "\n\t\t\t}\n";

        cout << "\n\tВключить логирование ошибок для данной директории? [yes/no]: ";
        getline(cin, celog_st);
        if (celog_st != "")
        {
            if (celog_st == "yes")
            {
                // nloc.custom_logs_status.push_back("error_log on;");

                cout << "\n\t\tУкажите каталог для хранения логов ошибок [default]: ";
                getline(cin, celog_path);
                celog_path = celog_path == "default" ? "/etc/nginx/logs/" : celog_path;

                cout << "\n\t\tУкажите имя логов: ";
                getline(cin, celog_name);

                cout << "\n\t\tУкажите формат логов ошибок [default]: ";
                getline(cin, celog_type);
                celog_type = celog_type == "default" ? "main" : celog_type;

                nloc.custom_logs_settings.push_back(
                    "error_log " + celog_path + (celog_st.back() == '/' ? "" : "/") +
                    celog_name + " " + celog_st + ";");
            }
            if (celog_st == "no")
                nloc.custom_logs_status.push_back("error_log off;");
        }

        cout << "\n\tВключить логирование доступа для данной директории? [yes/no]: ";
        getline(cin, celog_st);
        if (celog_st != "")
        {
            if (celog_st == "yes")
            {
                // nloc.custom_logs_status.push_back("access_log on;");

                cout << "\n\t\tУкажите каталог для хранения логов ошибок [default]: ";
                getline(cin, celog_path);
                celog_path = celog_path == "default" ? "/etc/nginx/logs/" : celog_path;

                cout << "\n\t\tУкажите имя логов: ";
                getline(cin, celog_name);

                cout << "\n\t\tУкажите формат логов ошибок [default]: ";
                getline(cin, celog_type);
                celog_type = celog_type == "default" ? "main" : celog_type;

                nloc.custom_logs_settings.push_back(
                    "error_log " + celog_path + (celog_st.back() == '/' ? "" : "/") +
                    celog_name + " " + celog_st + ";");
            }
            if (celog_st == "no")
                nloc.custom_logs_status.push_back("access_log off;");
        }

        cout << "\n\tВключить базовую аутентификацию? [yes/no]: ";
        getline(cin, auth_st);
        if (auth_st == "yes")
        {
            cout << "\n\t\t\tУкажите файл с учетными данными: ";
            getline(cin, auth_path);
            nloc.auth.push_back(R"(auth_basic "Restricted access";)");
            nloc.auth.push_back("auth_basic_user_file " + auth_path + ";");
        }

        cout << "\n\tВключить кэширование на стороне клиента? [yes/no]: ";
        getline(cin, exp_st);
        if (auth_st == "yes")
        {
            cout << "\n\t\tУкажите время валидности кэша: ";
            getline(cin, exp_st);
            nloc.expires = "expires " + auth_st + ";";
        }

        if (config.http.fast_cgi.size() > 0)
        {
            cout << "\n\tВключить кэширование FastCGI? [yes/no]: ";
            getline(cin, cgi_st);
            if (auth_st == "yes")
            {
                cout << "\n\t\tУкажите зону ключей кэша FastCGI: ";
                getline(cin, cgi_st);
                nloc.fast_cgi.push_back("fastcgi_cache " + cgi_st + ";");
                cout << "\n\t\tУкажите время валидности кэша FastCGI: ";
                getline(cin, cgi_st);
                nloc.fast_cgi.push_back("fastcgi_cache_valid 200  " + cgi_st + ";");
                cout << "\n\t\tУкажите прокси сервер FastCGI: ";
                getline(cin, cgi_st);
                if (cgi_st != "" && cgi_st != "no")
                    nloc.fast_cgi.push_back("fastcgi_pass " + cgi_st + ";");
                cout << "\n\t\tИспользовать дополнительные правила кэширования FastCGI? [yes/no]: ";
                getline(cin, cgi_st);
                if (cgi_st == "yes")
                    nloc.fast_cgi.push_back("fastcgi_no_cache $no_cache;");
            }
        }

        cout << "\n\tДобавить кастомные хэдеры? [yes/no]: ";
        getline(cin, prxhc_st);
        if (prxhc_st == "yes")
        {
            cnt = 1;
            cout << "\n\t\tУкажите хэдер " << cnt << ": ";
            cout << "\n\t\t\tИмя: ";
            getline(cin, hd_name);
            while (hd_name != "no")
            {
                cout << "\n\t\t\tДанные: ";
                getline(cin, hd_status);
                nloc.custom_headers.push_back(
                    "add_header " + hd_name + " " + hd_status + ";");
                cnt++;
                cout << "\n\t\tУкажите хэдер " << cnt << ": ";
                cout << "\n\t\t\tИмя: ";
                getline(cin, hd_name);
            }
        }

        if (config.http.load_balancer.size() > 0)
        {
            cout << "\n\tИспользовать балансировщик? [yes/no]: ";
            getline(cin, bal_st);
            if (bal_st == "yes")
            {
                cout << "\n\t\tУкажите имя группы серверов: ";
                getline(cin, bal_st);
                nloc.proxy_pass = "http://" + bal_st + ";";
            }
        }

        cnt = 1;
        cout << "\n\tУкажите дополнительные настройки для блока server [no чтобы закончить ввод]: ";
        cout << "\n\t\tДоп. настройка " << cnt << ": ";
        getline(cin, custom);
        while (custom != "no")
        {
            nloc.custom.push_back(custom + (custom.back() == ';' ? "" : ";"));
            cnt++;
            cout << "\n\t\tДоп. настройка " << cnt << ": ";
            getline(cin, custom);
        }

        config.locations.push_back(nloc);

        cout << "\n\tСоздать новую локацию? [yes/no]: ";
        getline(cin, loc_st);
    }
    cout << "\n\t\t\t\t[Конец конфигурации блоков locations]\n";
}

void pushAllBlocks()
{
    ofstream config_file(config.location + (config.location.back() == '/' ? "" : "/") + "nginx.conf");

    config_file << config.events.start;

    for (auto params : config.events.workers)
        if (params != "")
            config_file << config.events.tab + params;

    for (auto params : config.events.multi)
        if (params != "")
            config_file << config.events.tab + params;

    for (auto params : config.events.mutex)
        if (params != "")
            config_file << config.events.tab + params;

    for (auto params : config.events.custom)
        if (params != "")
            config_file << config.events.tab + params;

    config_file << config.events.end;

    config_file << config.http.start;

    for (auto params : config.http.include)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.global_headers)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.log_formatting)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.limit_concurrency)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.fast_cgi)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.load_balancer)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.geoip)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.geoip_blocks_allow)
        if (params != "")
            config_file << config.http.tab + params;

    for (auto params : config.http.geoip_blocks_deny)
        if (params != "")
            config_file << config.http.tab + "# " + params;

    config_file << config.server.start;

    for (auto params : config.server.listen)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.allow)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.deny)
        if (params != "")
            config_file << config.server.tab + params;

    config_file << config.server.server_name;

    config_file << config.server.root_folder;

    config_file << config.server.server_token;

    for (auto params : config.server.global_logs_status)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.global_logs_settings)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.ssl)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.buffers)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.timeouts)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.keepalive)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.gzip)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.caching)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto params : config.server.custom)
        if (params != "")
            config_file << config.server.tab + params;

    for (auto location : config.locations)
    {
        config_file << location.start;
        for (auto params : location.custom_logs_status)
            if (params != "")
                config_file << location.tab + params;

        for (auto params : location.custom_logs_settings)
            if (params != "")
                config_file << location.tab + params;

        for (auto params : location.auth)
            if (params != "")
                config_file << location.tab + params;

        for (auto params : location.fast_cgi)
            if (params != "")
                config_file << location.tab + params;

        if (location.expires != "")
            config_file << location.tab + location.expires;

        for (auto params : location.custom_headers)
            if (params != "")
                config_file << location.tab + params;
        
        if (location.proxy_pass != "")
            config_file << location.tab + location.proxy_pass;

        for (auto params : location.custom)
            if (params != "")
                config_file << location.tab + params;

        config_file << location.end;
    }

    config_file << config.server.end;

    for (auto params : config.http.custom)
        if (params != "")
            config_file << config.http.tab + params;

    config_file << config.http.end;

    config_file.close();
}

void applyConfiguration()
{
    string userInput;
    cout << "\n\tКакую конфигурацию необходимо применить? (default/preset/custom): ";
    cout << "\n\t\t[default] Применить стандартную конфигурацию для Ubuntu LTS 22.04";
    cout << "\n\t\t[preset]  Применить рекомендованную конфигурацию, разработанную в рамках курса";
    cout << "\n\t\t[custom]  Составить собственную конфигурацию";
    cout << "\n\n\tВаш выбор: ";
    getline(cin, userInput);

    if (userInput == "default")
    {
        ifstream srcFile("nginx.conf.ubuntu_template", ios::binary);
        ofstream dstFile(config.location + (config.location.back() == '/' ? "" : "/") + "nginx.conf", ios::binary);

        if (srcFile && dstFile)
        {
            dstFile << srcFile.rdbuf();
            cout << "\n\tФайл конфигурации успешно создан.";
        }
        else
            cout << "\n\tНе удалось создать файл конфигурации.";
    }
    else if (userInput == "preset")
    {
        ifstream srcFile("nginx.conf.mine", ios::binary);
        ofstream dstFile("nginx.conf", ios::binary);

        if (srcFile && dstFile)
        {
            dstFile << srcFile.rdbuf();
            cout << "\n\tФайл конфигурации успешно создан.";
        }
        else
            cout << "\n\tНе удалось создать файл конфигурации.";
    }
    else if (userInput == "custom")
    {
        ofstream customFile("nginx.conf");

        if (customFile)
        {
            cout << "\n\tДавайте приступим к персональной настройке файла конфигурации!";
            setEventsBlock();
            setHttpBlock();
            setServerBlock();
            setLocations();
            pushAllBlocks();
        }
        else
            cout << "\n\tНе удалось создать файл конфигурации.";
    }
    else
        cout << "\n\tНекорректный ввод.";
}

int main()
{
    cout << "\n\tПриветствуем в конфигураторе nginx!\n\n\tУкажите желаемое расположение nginx.conf:";
    // cin >> config.location;
    cout << "\n\t";

    config.location = "/home/alse0722/Desktop/kurs4/pract";
    applyConfiguration();

    return 0;
}