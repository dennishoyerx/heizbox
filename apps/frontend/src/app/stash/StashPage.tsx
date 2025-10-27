// File: apps/frontend/src/app/stash/StashPage.tsx
import { useState } from 'react'
import { Flex, Text, Button, Table, Badge, Dialog } from '@radix-ui/themes'
import { Plus, Minus, Trash } from '@phosphor-icons/react'
import { useStash } from '../hooks/useStash'
import { AddStashItemDialog } from './AddStashItemDialog'
import { WithdrawDialog } from './WithdrawDialog'
import { StashHistory } from './StashHistory'
import type { StashItemWithHistory } from '@heizbox/types'

function StashPage() {
	const { data, isLoading, isError, error, deleteItem, isDeleting } = useStash()
	const [isAddDialogOpen, setIsAddDialogOpen] = useState(false)
	const [withdrawDialogItem, setWithdrawDialogItem] = useState<StashItemWithHistory | null>(null)
	const [deleteConfirmItem, setDeleteConfirmItem] = useState<StashItemWithHistory | null>(null)
	console.log('data', data)
	const handleDelete = async (id: string) => {
		try {
			await deleteItem(id)
			setDeleteConfirmItem(null)
		} catch (err) {
			console.error('Error deleting item:', err)
		}
	}

	if (isLoading) {
		return (
			<Flex direction='column' gap='3' maxWidth='900px' className='mx-auto p-4'>
				<Text>Lade Stash-Daten...</Text>
			</Flex>
		)
	}

	if (isError) {
		return (
			<Flex direction='column' gap='3' maxWidth='900px' className='mx-auto p-4'>
				<Text color='red'>Fehler beim Laden: {error?.message}</Text>
			</Flex>
		)
	}

	return (
		<Flex direction='column' gap='4' maxWidth='900px' className='mx-auto p-4'>
			<Flex justify='between' align='center'>
				<div>
					<Text size='6' weight='bold'>
						Stash
					</Text>
					{data && (
						<Text size='2' color='gray'>
							Gesamt: {data.total_current?.toFixed(1) || 0}g verfügbar | {data.total_withdrawn?.toFixed(1) || 0}g
							entnommen
						</Text>
					)}
				</div>
				<Button onClick={() => setIsAddDialogOpen(true)}>
					<Plus size={16} /> Hinzufügen
				</Button>
			</Flex>

			{data && data.items.length === 0 ? (
				<Flex direction='column' align='center' gap='2' className='py-12'>
					<Text size='4' color='gray'>
						Noch keine Items im Stash
					</Text>
					<Button variant='soft' onClick={() => setIsAddDialogOpen(true)}>
						<Plus size={16} /> Erstes Item hinzufügen
					</Button>
				</Flex>
			) : (
				<Table.Root variant='surface'>
					<Table.Header>
						<Table.Row>
							<Table.ColumnHeaderCell>Sorte</Table.ColumnHeaderCell>
							<Table.ColumnHeaderCell>Verfügbar</Table.ColumnHeaderCell>
							<Table.ColumnHeaderCell>Start</Table.ColumnHeaderCell>
							<Table.ColumnHeaderCell>Entnommen</Table.ColumnHeaderCell>
							<Table.ColumnHeaderCell>Entnahmen</Table.ColumnHeaderCell>
							<Table.ColumnHeaderCell>Aktionen</Table.ColumnHeaderCell>
						</Table.Row>
					</Table.Header>

					<Table.Body>
						{data?.items.map((item) => {
							const progress = (item.current_amount / item.initial_amount) * 100
							const progressColor = progress > 50 ? 'green' : progress > 20 ? 'yellow' : 'red'

							return (
								<Table.Row key={item.id}>
									<Table.Cell>
										<Text weight='bold'>{item.name}</Text>
									</Table.Cell>
									<Table.Cell>
										<Flex direction='column' gap='1'>
											<Text weight='bold'>{item.current_amount.toFixed(1)}g</Text>
											<Badge color={progressColor} variant='soft' size='1'>
												{progress.toFixed(0)}%
											</Badge>
										</Flex>
									</Table.Cell>
									<Table.Cell>
										<Text color='gray'>{item.initial_amount.toFixed(1)}g</Text>
									</Table.Cell>
									<Table.Cell>
										<Text>{item.total_withdrawn.toFixed(1)}g</Text>
									</Table.Cell>
									<Table.Cell>
										<Text>{item.withdrawal_count}x</Text>
									</Table.Cell>
									<Table.Cell>
										<Flex gap='2'>
											<Button
												size='1'
												variant='soft'
												color='orange'
												onClick={() => setWithdrawDialogItem(item)}
												disabled={item.current_amount <= 0}
											>
												<Minus size={14} /> Entnahme
											</Button>
											<Button size='1' variant='soft' color='red' onClick={() => setDeleteConfirmItem(item)}>
												<Trash size={14} />
											</Button>
										</Flex>
									</Table.Cell>
								</Table.Row>
							)
						})}
					</Table.Body>
				</Table.Root>
			)}

			<StashHistory />

			<AddStashItemDialog isOpen={isAddDialogOpen} onClose={() => setIsAddDialogOpen(false)} />

			{withdrawDialogItem && (
				<WithdrawDialog item={withdrawDialogItem} isOpen={true} onClose={() => setWithdrawDialogItem(null)} />
			)}

			{deleteConfirmItem && (
				<Dialog.Root open={true} onOpenChange={() => setDeleteConfirmItem(null)}>
					<Dialog.Content style={{ maxWidth: 450 }}>
						<Dialog.Title>Item löschen?</Dialog.Title>
						<Dialog.Description size='2' mb='4'>
							Möchtest du "{deleteConfirmItem.name}" wirklich löschen? Alle Entnahme-Daten bleiben erhalten.
						</Dialog.Description>

						<Flex gap='3' justify='end'>
							<Dialog.Close>
								<Button variant='soft' color='gray'>
									Abbrechen
								</Button>
							</Dialog.Close>
							<Button color='red' onClick={() => handleDelete(deleteConfirmItem.id)} disabled={isDeleting}>
								Löschen
							</Button>
						</Flex>
					</Dialog.Content>
				</Dialog.Root>
			)}
		</Flex>
	)
}

export default StashPage
