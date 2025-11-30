#if defined _baidugeo_included
 #endinput
#endif
#define _baidugeo_included

/**
 * Query player's geolocation information (asynchronous)
 * 
 * This function automatically retrieves the player's IP and adds the query request 
 * to an asynchronous queue without blocking the main thread.
 * Query results are returned through forward callbacks.
 * 
 * @param client        Client index
 * 
 * @return              Returns true if successfully added to queue, false if client is invalid or IP is invalid
 * 
 * @note                Automatically retrieves and validates client IP address format
 * @note                If IP is already cached, callback will still be triggered but uses cached data
 * @note                Triggers BaiduGeo_OnLocationReceived forward on successful query
 * @note                Triggers BaiduGeo_OnLocationError forward on failed query
 * @note                Uses multiple worker threads for concurrent processing, max 4 simultaneous requests
 * @note                Request queue maximum is 100, returns error when exceeded
 */
native bool BaiduGeo_QueryPlayerLocation(int client);

/**
 * Get IP geolocation information from cache
 * 
 * This function only queries memory cache and will not initiate network requests.
 * 
 * @param ip            IPv4 address string to query
 * @param country       Buffer to store country name
 * @param countryLen    Maximum length of country buffer
 * @param province      Buffer to store province name
 * @param provinceLen   Maximum length of province buffer
 * @param city          Buffer to store city name
 * @param cityLen       Maximum length of city buffer
 * @param district      Buffer to store district name
 * @param districtLen   Maximum length of district buffer
 * 
 * @return              Returns true if IP exists in cache, false otherwise
 * 
 * @note                Cache uses LRU (Least Recently Used) strategy, old data will be automatically cleaned
 * @note                Cached data comes from previous successful query results
 * @note                Cache automatically expires, default retention time is 24 hours
 * 
 * Example:
 * char country[64], province[64], city[64], district[64];
 * if (BaiduGeo_GetCachedLocation(ip, country, sizeof(country), 
 *                                 province, sizeof(province),
 *                                 city, sizeof(city),
 *                                 district, sizeof(district))) {
 *     PrintToServer("Location: %s %s %s %s", country, province, city, district);
 * }
 */
native bool BaiduGeo_GetCachedLocation(const char[] ip,
                                        char[] country, int countryLen,
                                        char[] province, int provinceLen,
                                        char[] city, int cityLen,
                                        char[] district, int districtLen);

/**
 * Set Baidu Maps API key
 * 
 * @param apikey        Baidu Maps API key string
 * 
 * @noreturn
 * 
 * @note                Must set API key before querying, otherwise queries will fail
 * @note                API key will be automatically URL encoded
 * @note                Can update API key at any time
 * @note                Get API key: https://lbsyun.baidu.com/apiconsole/key
 * 
 * Example:
 * BaiduGeo_SetAPIKey("your_baidu_map_api_key_here");
 */
native void BaiduGeo_SetAPIKey(const char[] apikey);

/**
 * Check if extension is ready
 * 
 * @return              Returns true if API key is set, false otherwise
 * 
 * @note                Returns false when extension is loaded successfully but API key is not set
 * 
 * Example:
 * if (!BaiduGeo_IsAvailable()) {
 *     LogError("BaiduGeo extension API key not set");
 * }
 */
native bool BaiduGeo_IsAvailable();

/**
 * Clear cache data
 * 
 * @param ip            IP address to clear, pass empty string ("") to clear all cache
 * 
 * @noreturn
 * 
 * @note                Clear single IP: BaiduGeo_ClearCache("1.2.3.4");
 * @note                Clear all cache: BaiduGeo_ClearCache("");
 * 
 * Example:
 * // Clear single IP cache
 * BaiduGeo_ClearCache("114.114.114.114");
 * 
 * // Clear all cache
 * BaiduGeo_ClearCache("");
 */
native void BaiduGeo_ClearCache(const char[] ip = "");

/**
 * Set API request timeout
 * 
 * @param timeout       Timeout in seconds, default is 5 seconds
 * 
 * @noreturn
 * 
 * @note                Timeout includes connection timeout and total timeout
 * @note                Recommended to set between 3-10 seconds
 * 
 * Example:
 * BaiduGeo_SetTimeout(8); // Set 8 seconds timeout
 */
native void BaiduGeo_SetTimeout(int timeout);

/**
 * Get current cache entry count
 * 
 * @return              Total number of IP entries in cache
 * 
 * Example:
 * int count = BaiduGeo_GetCacheCount();
 * PrintToServer("Currently cached %d IP addresses", count);
 */
native int BaiduGeo_GetCacheCount();

/**
 * Check if specified IP is already cached
 * 
 * @param ip            IPv4 address to check
 * 
 * @return              Returns true if IP is cached, false otherwise
 * 
 * Example:
 * if (BaiduGeo_IsCached(ip)) {
 *     // Use cached data, no need to re-query
 *     char country[64], province[64], city[64], district[64];
 *     BaiduGeo_GetCachedLocation(ip, country, sizeof(country),
 *                                province, sizeof(province),
 *                                city, sizeof(city),
 *                                district, sizeof(district));
 * } else {
 *     // Initiate new query
 *     BaiduGeo_QueryPlayerLocation(client);
 * }
 */
native bool BaiduGeo_IsCached(const char[] ip);

/**
 * Callback when geolocation query succeeds
 * 
 * @param client        Client index passed during query
 * @param ip            Queried IP address
 * @param country       Country name (e.g., "China")
 * @param province      Province name (e.g., "Beijing")
 * @param city          City name (e.g., "Beijing")
 * @param district      District name (e.g., "Chaoyang District")
 * 
 * @noreturn
 * 
 * @note                This forward is called in the main thread, processes callback queue every 0.1 seconds
 * @note                Results are automatically added to cache
 * @note                Domestic IPs usually return detailed addresses, foreign IPs may only have country information
 * @note                District parameter may be empty, depending on API returned data
 * 
 * Example:
 * public void BaiduGeo_OnLocationReceived(int client, const char[] ip,
 *                                          const char[] country,
 *                                          const char[] province,
 *                                          const char[] city,
 *                                          const char[] district) {
 *     if (IsValidClient(client)) {
 *         PrintToChat(client, "Your location: %s %s %s %s", country, province, city, district);
 *     }
 * }
 */
forward void BaiduGeo_OnLocationReceived(int client, const char[] ip, 
                                          const char[] country, 
                                          const char[] province, 
                                          const char[] city,
                                          const char[] district);

/**
 * Callback when geolocation query fails
 * 
 * @param client        Client index passed during query
 * @param ip            Queried IP address
 * @param errorCode     Error code (currently always -1)
 * @param errorMsg      Error message description
 * 
 * @noreturn
 * 
 * @note                Common errors:
 *                      - "Invalid IPv4 address": IP address format is invalid
 *                      - "Query queue limit reached": Query queue is full
 *                      - "API Key not set": API key not set
 *                      - "libcurl not available": libcurl functionality unavailable
 *                      - "Failed to init cURL": cURL initialization failed
 *                      - "Parse error": JSON parsing failed (API return format error)
 *                      - cURL error messages: Network request failed (timeout, DNS resolution failed, etc.)
 * 
 * Example:
 * public void BaiduGeo_OnLocationError(int client, const char[] ip,
 *                                       int errorCode, const char[] errorMsg) {
 *     if (IsValidClient(client)) {
 *         LogError("Query failed for player %N IP %s: %s", client, ip, errorMsg);
 *     }
 * }
 */
forward void BaiduGeo_OnLocationError(int client, const char[] ip, 
                                       int errorCode, const char[] errorMsg);

public Extension __ext_baidugeo = 
{
    name = "BaiduGeo",
    file = "baidugeo.ext",
#if defined AUTOLOAD_EXTENSIONS
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public void __ext_baidugeo_SetNTVOptional()
{
    MarkNativeAsOptional("BaiduGeo_QueryPlayerLocation");
    MarkNativeAsOptional("BaiduGeo_GetCachedLocation");
    MarkNativeAsOptional("BaiduGeo_SetAPIKey");
    MarkNativeAsOptional("BaiduGeo_IsAvailable");
    MarkNativeAsOptional("BaiduGeo_ClearCache");
    MarkNativeAsOptional("BaiduGeo_SetTimeout");
    MarkNativeAsOptional("BaiduGeo_GetCacheCount");
    MarkNativeAsOptional("BaiduGeo_IsCached");
}
#endif
