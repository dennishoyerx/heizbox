import { Text, Flex } from '@radix-ui/themes'
import { Fire } from '@phosphor-icons/react'
import { useSession } from './useSession'

interface SessionHeaderProps {
	isHeating: boolean
	consumed: number
}

const SessionHeader: React.FC<SessionHeaderProps> = ({ isHeating, consumed }) => {
	const { data: session, isLoading, isError, error } = useSession()

	const heatCircleClass = isHeating
		? 'relative flex h-48 w-48 items-center justify-center rounded-full bg-gradient-to-br from-red-600 via-orange-500 to-amber-400 shadow-[0_0_60px_10px_rgba(255,100,0,0.7)] animate-pulse'
		: 'relative flex h-48 w-48 items-center justify-center rounded-full bg-gradient-to-br from-slate-700 via-slate-800 to-slate-900 shadow-[0_0_40px_rgba(0,0,0,0.5)]'

	const heatGlowClass = isHeating
		? 'absolute inset-0 rounded-full bg-[radial-gradient(circle_at_center,rgba(255,200,100,0.8),transparent)] blur-2xl animate-ping'
		: 'absolute inset-0 rounded-full bg-[radial-gradient(circle_at_center,rgba(100,100,100,0.2),transparent)] blur-xl'
	return (
		<Flex direction='column' align='center' gap='4' className='mt-8 mb-4 relative'>
			<div id='heat-circle' className={heatCircleClass}>
				{session && isHeating && (
					<>
						<div id='heat-glow' className={heatGlowClass}></div>
						<Fire
							size='96px'
							className='text-orange-50 drop-shadow-sm absolute text-white top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2'
						/>
					</>
				)}
				{session && !isHeating && (
					<Flex direction='column' align='center' className='absolute text-white'>
						<Text size='8' weight='bold' color='white'>
							{session.clicks}
						</Text>
					</Flex>
				)}
			</div>
			<Flex direction='column' align='start' className='absolute left-0 bottom-0'>
				<Text className='opacity-50'>Heute</Text>
				<Text>{consumed}g</Text>
			</Flex>
			<Flex direction='column' gap='2' className='absolute right-0 bottom-0'>
				<Flex direction='column' align='end'>
					<Text className='opacity-50'>Caps</Text>
					<Text>{session?.caps}</Text>
				</Flex>
				<Flex direction='column' align='end'>
					<Text className='opacity-50'>Session</Text>
					<Text>{session?.consumption}g</Text>
				</Flex>
			</Flex>
			{isError && <Text color='red'>Error loading session: {error.message}</Text>}
		</Flex>
	)
}

export default SessionHeader
