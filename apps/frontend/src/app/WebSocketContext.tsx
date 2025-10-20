import { createContext, useContext, useEffect, useState, useRef, useCallback, ReactNode } from 'react'
import type { ServerWebSocketMessage, ClientWebSocketMessage, DeviceStatusPayload } from '@heizbox/types'

export type WebSocketEvent = ServerWebSocketMessage
type EventListener = (event: WebSocketEvent) => void

interface WebSocketContextValue {
	isConnected: boolean
	deviceIsOn: boolean
	deviceIsHeating: boolean
	addEventListener: (listener: EventListener) => () => void
	sendMessage: (message: ClientWebSocketMessage) => void
}

const WebSocketContext = createContext<WebSocketContextValue | null>(null)

export const useWebSocket = () => {
	const context = useContext(WebSocketContext)
	if (!context) throw new Error('useWebSocket must be used within WebSocketProvider')
	return context
}

export const useWebSocketEvent = <T extends WebSocketEvent['type']>(
	eventType: T,
	handler: (event: Extract<WebSocketEvent, { type: T }>) => void,
) => {
	const { addEventListener } = useWebSocket()
	useEffect(() => {
		return addEventListener((event) => {
			if (event.type === eventType) handler(event as Extract<WebSocketEvent, { type: T }>)
		})
	}, [addEventListener, eventType, handler])
}

interface WebSocketProviderProps {
	children: ReactNode
	deviceId?: string
}

export const WebSocketProvider = ({ children, deviceId = 'my-esp32-device' }: WebSocketProviderProps) => {
	const [isConnected, setIsConnected] = useState(false)
	const [deviceIsOn, setDeviceIsOn] = useState(false)
	const [deviceIsHeating, setDeviceIsHeating] = useState(false)

	const listeners = useRef<Set<EventListener>>(new Set())
	const wsRef = useRef<WebSocket | null>(null)
	const reconnectRef = useRef<NodeJS.Timeout | null>(null)
	const statusUpdateTimeout = useRef<NodeJS.Timeout | null>(null)
	const pendingStatus = useRef<{ isOn?: boolean; isHeating?: boolean }>({})

	// Listener Management
	const addEventListener = useCallback((listener: EventListener) => {
		listeners.current.add(listener)
		return () => listeners.current.delete(listener)
	}, [])

	// Send Message
	const sendMessage = useCallback((message: ClientWebSocketMessage) => {
		const ws = wsRef.current
		if (ws && ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(message))
	}, [])

	// Fetch initial device status
	useEffect(() => {
		const fetchInitialStatus = async () => {
			try {
				const backendBaseUrl = import.meta.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787'
				const res = await fetch(`${backendBaseUrl}/api/device-status/${deviceId}/status`)
				if (res.ok) {
					const status = (await res.json()) as DeviceStatusPayload
					setDeviceIsOn(status.isOn)
					setDeviceIsHeating(status.isHeating)
				}
			} catch (err) {
				console.error('Error fetching initial device status:', err)
			}
		}
		fetchInitialStatus()
	}, [deviceId])

	// Debounced Status Update
	const queueStatusUpdate = useCallback((status: { isOn?: boolean; isHeating?: boolean }) => {
		Object.assign(pendingStatus.current, status)
		if (statusUpdateTimeout.current) clearTimeout(statusUpdateTimeout.current)
		statusUpdateTimeout.current = setTimeout(() => {
			if (pendingStatus.current.isOn !== undefined) setDeviceIsOn(pendingStatus.current.isOn)
			if (pendingStatus.current.isHeating !== undefined) setDeviceIsHeating(pendingStatus.current.isHeating)
			pendingStatus.current = {}
		}, 50) // 50ms debounce
	}, [])

	// WebSocket Connection & Reconnect
	useEffect(() => {
		let isUnmounted = false

		const connect = () => {
			if (isUnmounted) return

			const backendBaseUrl = import.meta.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787'
			const wsUrl = `${backendBaseUrl.replace('http', 'ws')}/ws?deviceId=${deviceId}&type=frontend`

			const ws = new WebSocket(wsUrl)
			wsRef.current = ws

			ws.onopen = () => {
				console.log('WebSocket connected')
				setIsConnected(true)
			}

			ws.onmessage = (event) => {
				try {
					const message = JSON.parse(event.data) as ServerWebSocketMessage

					// Broadcast to all listeners
					listeners.current.forEach((l) => l(message))

					// Debounced status updates
					if (message.type === 'statusUpdate') {
						queueStatusUpdate({
							isOn: message.isOn,
							isHeating: message.isHeating,
						})
					}
				} catch (err) {
					console.error('Failed to parse WS message:', err)
				}
			}

			ws.onclose = () => {
				console.log('WebSocket disconnected')
				setIsConnected(false)
				if (!isUnmounted) reconnectRef.current = setTimeout(connect, 3000)
			}

			ws.onerror = (err) => {
				console.error('WebSocket error:', err)
				ws.close() // triggers onclose
			}
		}

		connect()

		return () => {
			isUnmounted = true
			if (reconnectRef.current) clearTimeout(reconnectRef.current)
			if (statusUpdateTimeout.current) clearTimeout(statusUpdateTimeout.current)
			wsRef.current?.close()
		}
	}, [deviceId, queueStatusUpdate])

	const value: WebSocketContextValue = {
		isConnected,
		deviceIsOn,
		deviceIsHeating,
		addEventListener,
		sendMessage,
	}

	return <WebSocketContext.Provider value={value}>{children}</WebSocketContext.Provider>
}
