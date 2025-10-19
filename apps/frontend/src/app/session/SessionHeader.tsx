import { Text, Flex, Grid } from '@radix-ui/themes'
import { Fire } from '@phosphor-icons/react'
import { useSession } from './useSession'

interface SessionHeaderProps {
	isHeating: boolean
	consumed: number
}

const Stat = ({ label, value, suffix }: { label: string; value: number; suffix?: string }) => (
	<Flex direction='column' align='center'>
		<Text className='font-numeric text-3xl'>
			{value}
			{suffix}
		</Text>
		<Text className='opacity-50 text-xl'>{label}</Text>
	</Flex>
)
const SessionHeader: React.FC<SessionHeaderProps> = ({ isHeating, consumed }) => {
	const { data: session, isLoading, isError, error } = useSession()
	return (
		<Grid columns='4' className='py-4'>
			<Stat label='Caps' value={session?.caps || 0} />
			<Stat label='Clicks' value={session?.clicks || 0} />
			<Stat label='Session' value={session?.consumption || 0} suffix='g' />
			<Stat label='Heute' value={consumed} suffix='g' />
		</Grid>
	)
}

export default SessionHeader
