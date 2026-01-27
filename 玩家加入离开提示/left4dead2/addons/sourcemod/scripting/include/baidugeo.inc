#if defined _baidugeo_included
 #endinput
#endif
#define _baidugeo_included

/**
 * Query player's geolocation information (asynchronous)
 * * This function automatically retrieves the player's IP and adds the query request 
 * to an asynchronous queue without blocking the main thread.
 * Query results are returned through forward callbacks.
 * * @param client        Client index
 * * @return              Returns true if successfully added to queue, false if client or IP is invalid
 * * @note                Automatically retrieves and validates client IP address format
 * @note                Triggers BaiduGeo_OnLocationReceived forward on successful query
 * @note                Triggers BaiduGeo_OnLocationError forward on failed query
 * @note                Uses multiple worker threads for concurrent processing
 */
native bool BaiduGeo_QueryPlayerLocation(int client);

/**
 * Get IP geolocation information from cache
 * * This function only queries memory cache and will not initiate network requests.
 * * @param ip            IPv4 address string to query
 * @param country       Buffer to store country name
 * @param countryLen    Maximum length of country buffer
 * @param province      Buffer to store province name
 * @param provinceLen   Maximum length of province buffer
 * @param city          Buffer to store city name
 * @param cityLen       Maximum length of city buffer
 * * @return              Returns true if IP exists in cache, false otherwise
 * * @note                Cache uses LRU (Least Recently Used) strategy
 * @note                Cache automatically expires, default retention time is 24 hours
 * * Example:
 * char country[64], province[64], city[64];
 * if (BaiduGeo_GetCachedLocation(ip, country, sizeof(country), 
 * province, sizeof(province),
 * city, sizeof(city))) {
 * PrintToServer("Location: %s %s %s", country, province, city);
 * }
 */
native bool BaiduGeo_GetCachedLocation(const char[] ip, 
                                        char[] country, int countryLen, 
                                        char[] province, int provinceLen, 
                                        char[] city, int cityLen);

/**
 * Set Baidu Maps API key
 * * @param apikey        Baidu Maps API key string
 * * @noreturn
 * * @note                Must set API key before querying, otherwise queries will fail
 */
native void BaiduGeo_SetAPIKey(const char[] apikey);

/**
 * Check if extension is ready
 * * @return              Returns true if API key is set, false otherwise
 */
native bool BaiduGeo_IsAvailable();

/**
 * Clear cache data
 * * @param ip            IP address to clear, pass empty string ("") to clear all cache
 * * @noreturn
 */
native void BaiduGeo_ClearCache(const char[] ip = "");

/**
 * Set API request timeout
 * * @param timeout       Timeout in seconds, default is 5 seconds
 * * @noreturn
 */
native void BaiduGeo_SetTimeout(int timeout);

/**
 * Get current cache entry count
 * * @return              Total number of IP entries in cache
 */
native int BaiduGeo_GetCacheCount();

/**
 * Check if specified IP is already cached
 * * @param ip            IPv4 address to check
 * * @return              Returns true if IP is cached, false otherwise
 */
native bool BaiduGeo_IsCached(const char[] ip);

/**
 * Callback when geolocation query succeeds
 * * @param client        Client index passed during query
 * @param ip            Queried IP address
 * @param country       Country name (e.g., "China")
 * @param province      Province name (e.g., "Beijing")
 * @param city          City name (e.g., "Beijing")
 * * @noreturn
 * * @note                This forward is called in the main thread
 * @note                Results are automatically added to cache
 * * Example:
 * public void BaiduGeo_OnLocationReceived(int client, const char[] ip, 
 * const char[] country, 
 * const char[] province, 
 * const char[] city) {
 * if (IsClientInGame(client)) {
 * PrintToChat(client, "Your location: %s %s %s", country, province, city);
 * }
 * }
 */
forward void BaiduGeo_OnLocationReceived(int client, const char[] ip, 
                                          const char[] country, 
                                          const char[] province, 
                                          const char[] city);

/**
 * Callback when geolocation query fails
 * * @param client        Client index passed during query
 * @param ip            Queried IP address
 * @param errorCode     Error code (currently always -1)
 * @param errorMsg      Error message description
 * * @noreturn
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
