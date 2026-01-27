#if defined _baidugeo_included
 #endinput
#endif
#define _baidugeo_included

/**
 * 查询玩家地理位置信息（异步）
 * * 此函数自动获取玩家 IP 并将查询请求添加到异步队列，不会阻塞主线程。
 * 查询结果将通过 forward 回调函数返回。
 * * @param client        客户端索引
 * * @return              如果成功加入队列返回 true，如果客户端无效或 IP 无效则返回 false
 * * @note                自动获取并验证客户端 IP 地址格式
 * @note                如果 IP 已在缓存中，回调仍会被触发，但使用的是缓存数据
 * @note                查询成功时触发 BaiduGeo_OnLocationReceived
 * @note                查询失败时触发 BaiduGeo_OnLocationError
 * @note                使用多工作线程并发处理，最大支持 4 个并发请求
 * @note                请求队列最大长度为 100，超过时返回错误
 */
native bool BaiduGeo_QueryPlayerLocation(int client);

/**
 * 从缓存获取 IP 地理位置信息
 * * 此函数仅查询内存缓存，不会发起网络请求。
 * * @param ip            要查询的 IPv4 地址字符串
 * @param country       用于存储国家名称的缓冲区
 * @param countryLen    国家缓冲区的最大长度
 * @param province      用于存储省份名称的缓冲区
 * @param provinceLen   省份缓冲区的最大长度
 * @param city          用于存储城市名称的缓冲区
 * @param cityLen       城市缓冲区的最大长度
 * * @return              如果 IP 存在于缓存中返回 true，否则返回 false
 * * @note                缓存使用 LRU（最近最少使用）策略，旧数据会被自动清理
 * @note                缓存数据来源于之前成功的查询结果
 * @note                缓存会自动过期，默认保留时间为 24 小时
 * * 示例:
 * char country[64], province[64], city[64];
 * if (BaiduGeo_GetCachedLocation(ip, country, sizeof(country), 
 * province, sizeof(province),
 * city, sizeof(city))) {
 * PrintToServer("位置: %s %s %s", country, province, city);
 * }
 */
native bool BaiduGeo_GetCachedLocation(const char[] ip,
                                        char[] country, int countryLen,
                                        char[] province, int provinceLen,
                                        char[] city, int cityLen);

/**
 * 设置百度地图 API 密钥
 * * @param apikey        百度地图 API 密钥字符串
 * * @noreturn
 * * @note                必须在查询前设置 API 密钥，否则查询会失败
 * @note                API 密钥会自动进行 URL 编码
 * @note                可以随时更新 API 密钥
 * @note                获取 API 密钥: https://lbsyun.baidu.com/apiconsole/key
 * * 示例:
 * BaiduGeo_SetAPIKey("在此处填写您的百度地图API密钥");
 */
native void BaiduGeo_SetAPIKey(const char[] apikey);

/**
 * 检查扩展是否就绪
 * * @return              如果已设置 API 密钥返回 true，否则返回 false
 * * @note                当扩展加载成功但未设置 API 密钥时返回 false
 * * 示例:
 * if (!BaiduGeo_IsAvailable()) {
 * LogError("BaiduGeo 扩展未设置 API 密钥");
 * }
 */
native bool BaiduGeo_IsAvailable();

/**
 * 清除缓存数据
 * * @param ip            要清除的 IP 地址，传入空字符串 ("") 则清除所有缓存
 * * @noreturn
 * * @note                清除单个 IP: BaiduGeo_ClearCache("1.2.3.4");
 * @note                清除所有缓存: BaiduGeo_ClearCache("");
 * * 示例:
 * // 清除单个 IP 缓存
 * BaiduGeo_ClearCache("114.114.114.114");
 * * // 清除所有缓存
 * BaiduGeo_ClearCache("");
 */
native void BaiduGeo_ClearCache(const char[] ip = "");

/**
 * 设置 API 请求超时时间
 * * @param timeout       超时时间（秒），默认为 5 秒
 * * @noreturn
 * * @note                超时时间包含连接超时和总传输超时
 * @note                建议设置为 3-10 秒之间
 * * 示例:
 * BaiduGeo_SetTimeout(8); // 设置 8 秒超时
 */
native void BaiduGeo_SetTimeout(int timeout);

/**
 * 获取当前缓存条目数量
 * * @return              缓存中的 IP 条目总数
 * * 示例:
 * int count = BaiduGeo_GetCacheCount();
 * PrintToServer("当前缓存了 %d 个 IP 地址", count);
 */
native int BaiduGeo_GetCacheCount();

/**
 * 检查指定 IP 是否已缓存
 * * @param ip            要检查的 IPv4 地址
 * * @return              如果 IP 已缓存返回 true，否则返回 false
 * * 示例:
 * if (BaiduGeo_IsCached(ip)) {
 * // 使用缓存数据，无需重新查询
 * char country[64], province[64], city[64];
 * BaiduGeo_GetCachedLocation(ip, country, sizeof(country),
 * province, sizeof(province),
 * city, sizeof(city));
 * } else {
 * // 发起新查询
 * BaiduGeo_QueryPlayerLocation(client);
 * }
 */
native bool BaiduGeo_IsCached(const char[] ip);

/**
 * 地理位置查询成功时的回调
 * * @param client        查询时传入的客户端索引
 * @param ip            被查询的 IP 地址
 * @param country       国家名称 (例如: "中国")
 * @param province      省份名称 (例如: "北京市")
 * @param city          城市名称 (例如: "北京市")
 * * @noreturn
 * * @note                此转发在主线程中调用，每 0.1 秒处理一次回调队列
 * @note                结果会自动加入缓存
 * @note                国内 IP 通常返回详细地址，国外 IP 可能只有国家信息
 * * 示例:
 * public void BaiduGeo_OnLocationReceived(int client, const char[] ip,
 * const char[] country,
 * const char[] province,
 * const char[] city) {
 * if (IsValidClient(client)) {
 * PrintToChat(client, "您的位置: %s %s %s", country, province, city);
 * }
 * }
 */
forward void BaiduGeo_OnLocationReceived(int client, const char[] ip, 
                                          const char[] country, 
                                          const char[] province, 
                                          const char[] city);

/**
 * 地理位置查询失败时的回调
 * * @param client        查询时传入的客户端索引
 * @param ip            被查询的 IP 地址
 * @param errorCode     错误代码 (当前固定为 -1)
 * @param errorMsg      错误信息描述
 * * @noreturn
 * * @note                常见错误:
 * - "Invalid IPv4 address": IP 地址格式无效
 * - "Query queue limit reached": 查询队列已满
 * - "API Key not set": 未设置 API 密钥
 * - "Failed to init cURL": cURL 初始化失败
 * - "Parse error": JSON 解析失败 (API 返回格式错误)
 * - cURL 错误信息: 网络请求失败 (超时、DNS 解析失败等)
 * * 示例:
 * public void BaiduGeo_OnLocationError(int client, const char[] ip, 
 * int errorCode, const char[] errorMsg) {
 * if (IsValidClient(client)) {
 * LogError("玩家 %N IP %s 查询失败: %s", client, ip, errorMsg);
 * }
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