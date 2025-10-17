// File: apps/frontend/src/app/stash/WithdrawDialog.tsx
import { useState } from 'react';
import { Dialog, Flex, TextField, Button, Text, TextArea, Badge } from '@radix-ui/themes';
import { useStash } from './useStash';
import type { StashItemWithHistory } from '@heizbox/types';

interface WithdrawDialogProps {
  item: StashItemWithHistory;
  isOpen: boolean;
  onClose: () => void;
}

export const WithdrawDialog = ({ item, isOpen, onClose }: WithdrawDialogProps) => {
  const { withdrawItem, isWithdrawing } = useStash();
  const [amount, setAmount] = useState('');
  const [error, setError] = useState('');

  const handleSubmit = async () => {
    setError('');

    const amountNum = parseFloat(amount);
    if (isNaN(amountNum) || amountNum <= 0) {
      setError('Bitte eine gültige Menge eingeben');
      return;
    }

    if (amountNum > item.current_amount) {
      setError(`Nur ${item.current_amount.toFixed(1)}g verfügbar`);
      return;
    }

    try {
      await withdrawItem({
        itemId: item.id,
        data: {
          amount: amountNum,
        },
      });
      
      setAmount('');
      onClose();
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Fehler bei der Entnahme');
    }
  };

  // Quick-Select-Buttons
  const quickAmounts = [0.1, 0.2, 0.5, 1.0];

  return (
    <Dialog.Root open={isOpen} onOpenChange={onClose}>
      <Dialog.Content style={{ maxWidth: 450 }}>
        <Dialog.Title>Entnahme: {item.name}</Dialog.Title>
        <Dialog.Description size="2" mb="4">
          Verfügbar: <Badge color="green">{item.current_amount.toFixed(1)}g</Badge>
        </Dialog.Description>

        <Flex direction="column" gap="3">
          <label>
            <Text as="div" size="2" mb="1" weight="bold">
              Menge (Gramm)
            </Text>
            <TextField.Root
              type="number"
              step="0.1"
              placeholder="0.5"
              value={amount}
              onChange={(e) => setAmount(e.target.value)}
            />
          </label>

          <Flex gap="2" wrap="wrap">
            {quickAmounts.map((quickAmount) => (
              <Button
                key={quickAmount}
                size="1"
                variant="soft"
                onClick={() => setAmount(quickAmount.toString())}
                disabled={quickAmount > item.current_amount}
              >
                {quickAmount}g
              </Button>
            ))}
            <Button
              size="1"
              variant="soft"
              color="orange"
              onClick={() => setAmount(item.current_amount.toFixed(1))}
            >
              Alles
            </Button>
          </Flex>

          {error && (
            <Text color="red" size="2">{error}</Text>
          )}
        </Flex>

        <Flex gap="3" mt="4" justify="end">
          <Dialog.Close>
            <Button variant="soft" color="gray">Abbrechen</Button>
          </Dialog.Close>
          <Button onClick={handleSubmit} disabled={isWithdrawing} color="orange">
            {isWithdrawing ? 'Wird entnommen...' : 'Entnehmen'}
          </Button>
        </Flex>
      </Dialog.Content>
    </Dialog.Root>
  );
};