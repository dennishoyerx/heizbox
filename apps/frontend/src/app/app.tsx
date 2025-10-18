import { Routes, Route } from 'react-router-dom'
import Header from './components/Header'
import { UsagePage } from './usage'
import { SessionPage } from './session'
import { Box, Flex, Theme } from '@radix-ui/themes'
import { useWebSocket } from './WebSocketContext'
import { ConnectionStatus } from './components/ConnectionStatus'
import { useTheme } from './hooks/useTheme'
import StashPage from './stash/StashPage'
import StashStatsPage from './stash/StashStatsPage'
import { useQueryClient } from '@tanstack/react-query'
import { useEffect } from 'react'

function AppContent() {
	const { deviceIsOn, deviceIsHeating } = useWebSocket()
	const { theme, toggleTheme } = useTheme()
	const queryClient = useQueryClient()

	useEffect(() => {
		const handleVisibilityChange = () => {
			if (document.visibilityState === 'visible') {
				console.log('App became visible, invalidating queries.')
				queryClient.invalidateQueries({ queryKey: ['heatCycles'] })
				queryClient.invalidateQueries({ queryKey: ['runningSession'] })
				queryClient.invalidateQueries({ queryKey: ['stash'] })
				queryClient.invalidateQueries({ queryKey: ['stashStats'] })
				queryClient.invalidateQueries({ queryKey: ['statistics'] })
			}
		}

		document.addEventListener('visibilitychange', handleVisibilityChange)

		return () => {
			document.removeEventListener('visibilitychange', handleVisibilityChange)
		}
	}, [queryClient])

	return (
		<Theme appearance={theme} grayColor='slate' accentColor='orange' radius='medium'>
			<Box className='min-h-screen font-josefin'>
				<Flex direction='column' gap='4'>
					<Header
						deviceName='Heizbox'
						deviceStatus={deviceIsOn ? 'Online' : 'Offline'}
						heatingStatus={deviceIsHeating ? 'Heizt' : 'Inaktiv'}
						theme={theme}
						toggleTheme={toggleTheme}
					/>
					<main>
						<Routes>
							<Route path='/' element={<SessionPage />} />
							<Route path='/usage' element={<UsagePage />} />
							<Route path='/stash' element={<StashPage />} />
							<Route path='/stash/stats' element={<StashStatsPage />} />
						</Routes>
					</main>
				</Flex>
			</Box>
			<ConnectionStatus />
		</Theme>
	)
}

function App() {
	return <AppContent />
}

export default App
