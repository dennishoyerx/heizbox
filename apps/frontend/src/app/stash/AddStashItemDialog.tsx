// File: apps/frontend/src/app/stash/AddStashItemDialog.tsx
import { useState } from 'react'
import { Dialog, Flex, TextField, Button, Text, TextArea } from '@radix-ui/themes'
import { useStash } from '../hooks/useStash'

interface AddStashItemDialogProps {
	isOpen: boolean
	onClose: () => void
}

export const AddStashItemDialog = ({ isOpen, onClose }: AddStashItemDialogProps) => {
	const { createItem, isCreating } = useStash()
	const [itemName, setItemName] = useState('')
	const [quantity, setQuantity] = useState('')
	const [error, setError] = useState('')

	const handleSubmit = async () => {
		setError('')

		if (!itemName.trim()) {
			setError('Bitte einen Namen eingeben')
			return
		}

		const quantityNum = parseFloat(quantity)
		if (isNaN(quantityNum) || quantityNum <= 0) {
			setError('Bitte eine gültige Menge eingeben')
			return
		}

		try {
			await createItem({
				name: itemName.trim(),
				initial_amount: quantityNum,
			})

			// Reset form
			setItemName('')
			setQuantity('')
			onClose()
		} catch (err) {
			setError(err instanceof Error ? err.message : 'Fehler beim Erstellen')
		}
	}

	return (
		<Dialog.Root open={isOpen} onOpenChange={onClose}>
			<Dialog.Content style={{ maxWidth: 450 }}>
				<Dialog.Title>Neues Item hinzufügen</Dialog.Title>
				<Dialog.Description size='2' mb='4'>
					Füge ein neues Item zu deinem Stash hinzu.
				</Dialog.Description>

				<Flex direction='column' gap='3'>
					<label>
						<Text as='div' size='2' mb='1' weight='bold'>
							Sorte / Name
						</Text>
						<TextField.Root
							placeholder='z.B. Purple Haze'
							value={itemName}
							onChange={(e) => setItemName(e.target.value)}
						/>
					</label>

					<label>
						<Text as='div' size='2' mb='1' weight='bold'>
							Menge (Gramm)
						</Text>
						<TextField.Root
							type='number'
							step='0.1'
							placeholder='5.0'
							value={quantity}
							onChange={(e) => setQuantity(e.target.value)}
						/>
					</label>

					<label>
						<Text as='div' size='2' mb='1' weight='bold'>
							Notizen (optional)
						</Text>
					</label>

					{error && (
						<Text color='red' size='2'>
							{error}
						</Text>
					)}
				</Flex>

				<Flex gap='3' mt='4' justify='end'>
					<Dialog.Close>
						<Button variant='soft' color='gray'>
							Abbrechen
						</Button>
					</Dialog.Close>
					<Button onClick={handleSubmit} disabled={isCreating}>
						{isCreating ? 'Wird erstellt...' : 'Hinzufügen'}
					</Button>
				</Flex>
			</Dialog.Content>
		</Dialog.Root>
	)
}
