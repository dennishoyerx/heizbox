// File: apps/frontend/src/app/stash/WithdrawDialog.tsx
import { useState } from "react";
import {
  Dialog,
  Flex,
  TextField,
  Button,
  Text,
  TextArea,
  Badge,
} from "@radix-ui/themes";
import { useStash } from "../hooks/useStash";
import type { StashItemWithHistory } from "@heizbox/types";

interface WithdrawDialogProps {
  item: StashItemWithHistory;
  isOpen: boolean;
  onClose: () => void;
}

export const WithdrawDialog = ({
  item,
  isOpen,
  onClose,
}: WithdrawDialogProps) => {
  const { withdrawItem, isWithdrawing } = useStash();
  const [quantity, setQuantity] = useState("");
  const [notes, setNotes] = useState("");
  const [error, setError] = useState("");

  const handleSubmit = async () => {
    setError("");

    const quantityNum = parseFloat(quantity);
    if (isNaN(quantityNum) || quantityNum <= 0) {
      setError("Bitte eine gültige Menge eingeben");
      return;
    }

    if (quantityNum > item.current_amount) {
      setError(`Nur ${item.current_amount.toFixed(1)}g verfügbar`);
      return;
    }

    try {
      await withdrawItem({
        itemId: item.id,
        data: {
          quantity: quantityNum,
          notes: notes.trim() || undefined,
        },
      });

      setQuantity("");
      setNotes("");
      onClose();
    } catch (err) {
      setError(err instanceof Error ? err.message : "Fehler bei der Entnahme");
    }
  };

  // Quick-Select-Buttons
  const quickAmounts = [0.1, 0.2, 0.5, 1.0];

  return (
    <Dialog.Root open={isOpen} onOpenChange={onClose}>
      <Dialog.Content style={{ maxWidth: 450 }}>
        <Dialog.Title>Entnahme: {item.item_name}</Dialog.Title>
        <Dialog.Description size="2" mb="4">
          Verfügbar:{" "}
          <Badge color="green">{item.current_amount.toFixed(1)}g</Badge>
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
              value={quantity}
              onChange={(e) => setQuantity(e.target.value)}
            />
          </label>

          <Flex gap="2" wrap="wrap">
            {quickAmounts.map((amount) => (
              <Button
                key={amount}
                size="1"
                variant="soft"
                onClick={() => setQuantity(amount.toString())}
                disabled={amount > item.current_amount}
              >
                {amount}g
              </Button>
            ))}
            <Button
              size="1"
              variant="soft"
              color="orange"
              onClick={() => setQuantity(item.current_amount.toFixed(1))}
            >
              Alles
            </Button>
          </Flex>

          <label>
            <Text as="div" size="2" mb="1" weight="bold">
              Notizen (optional)
            </Text>
            <TextArea
              placeholder="z.B. Für Abendkonsum"
              value={notes}
              onChange={(e) => setNotes(e.target.value)}
              rows={2}
            />
          </label>

          {error && (
            <Text color="red" size="2">
              {error}
            </Text>
          )}
        </Flex>

        <Flex gap="3" mt="4" justify="end">
          <Dialog.Close>
            <Button variant="soft" color="gray">
              Abbrechen
            </Button>
          </Dialog.Close>
          <Button
            onClick={handleSubmit}
            disabled={isWithdrawing}
            color="orange"
          >
            {isWithdrawing ? "Wird entnommen..." : "Entnehmen"}
          </Button>
        </Flex>
      </Dialog.Content>
    </Dialog.Root>
  );
};
