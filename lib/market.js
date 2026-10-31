/**
 * Stub: no price source of the original pool is used for Conceal (CCX).
 * Вызывающий код (api.js, charts.js) корректно обрабатывает пустой ответ.
 **/
exports.get = function (source, tickers, callback) {
	callback([]);
};
