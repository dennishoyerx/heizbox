import { useState, useEffect } from 'react'
import { Popover, Theme, Flex, Box, Text } from '@radix-ui/themes'
import { Terminal, CircleNotch } from '@phosphor-icons/react'
import { fetchLogs } from '../../api'

interface LogEntry {
	timestamp: number
	log_type: string
	message: string
}

export function LogPopover() {
	const [logs, setLogs] = useState<LogEntry[]>([])
	const [isLoading, setIsLoading] = useState(false)
	const [isOpen, setIsOpen] = useState(false)

	useEffect(() => {
		if (isOpen) {
			setIsLoading(true)
			fetchLogs(10)
				.then(setLogs)
				.catch(console.error)
				.finally(() => setIsLoading(false))
		}
	}, [isOpen])

	const formatTime = (timestamp: number) => {
		return new Date(timestamp).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false })
	}

	return (
		<Popover.Root onOpenChange={setIsOpen}>
			<Popover.Trigger>
				<Terminal size={24} className='cursor-pointer' />
			</Popover.Trigger>
			<Popover.Content maxWidth={'800px'} width={'100%'}>
				<Theme>
					<Box>
						<Text as='div' size='2' weight='bold' mb='2'>
							Last 10 Logs
						</Text>

						{isLoading ? (
							<Flex justify='center' align='center' style={{ height: '100px' }}>
								<CircleNotch size={32} className='animate-spin' />
							</Flex>
						) : (
							<Flex direction='column' gap='2'>
								{logs.map((log, index) => (
									<Flex key={index} gap='3' align='center'>
										<Text size='1' color='gray'>
											{formatTime(log.timestamp)}
										</Text>
										<Text size='1' weight='bold' style={{ width: '100px' }}>
											[{log.log_type}]
										</Text>
										<Text size='1'>{log.message}</Text>
									</Flex>
								))}
							</Flex>
						)}
					</Box>
				</Theme>
			</Popover.Content>
		</Popover.Root>
	)
}
