export const config = {
	cors: {
		allowedOrigins: ['.hzbx.de', 'hzbx.de', 'localhost:5173', '127.0.0.1:5173'],
	},
	sentry: {
		dsn: 'https://4d0dd22715fe1480def3e8d645856914@o4510082700345344.ingest.de.sentry.io/4510173369925712',
	},
	session: {
		groupIntervalMinutes: 60,
		consumptionPerCycle: 0.05,
	},
	deviceStatus: {
		offlineThresholdMs: 90000,
	},
} as const
