// File: apps/frontend/src/app/stash/useStash.ts
import { useQuery, useMutation, useQueryClient } from "@tanstack/react-query";
import {
  fetchStashItems,
  createStashItem,
  withdrawStashItem,
  deleteStashItem,
} from "@/api";
import { useWebSocketEvent } from "../WebSocketContext";
import { useCallback } from "react";
import type {
  CreateStashItemRequest,
  WithdrawStashItemRequest,
} from "@heizbox/types";

export const useStash = () => {
  const queryClient = useQueryClient();

  const query = useQuery({
    queryKey: ["stash"],
    queryFn: fetchStashItems,
  });

  const createMutation = useMutation({
    mutationFn: (data: CreateStashItemRequest) => createStashItem(data),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["stash"] });
    },
  });

  const withdrawMutation = useMutation({
    mutationFn: ({
      itemId,
      data,
    }: {
      itemId: string;
      data: WithdrawStashItemRequest;
    }) => withdrawStashItem(itemId, data),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["stash"] });
    },
  });

  const deleteMutation = useMutation({
    mutationFn: (itemId: string) => deleteStashItem(itemId),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["stash"] });
    },
  });

  // WebSocket-Event-Handler
  const handleStashUpdate = useCallback(() => {
    console.log("Stash update received, invalidating query...");
    queryClient.invalidateQueries({ queryKey: ["stash"] });
  }, [queryClient]);

  useWebSocketEvent("stashUpdated", handleStashUpdate);

  return {
    data: query.data,
    isLoading: query.isLoading,
    isError: query.isError,
    error: query.error,
    createItem: createMutation.mutateAsync,
    withdrawItem: withdrawMutation.mutateAsync,
    deleteItem: deleteMutation.mutateAsync,
    isCreating: createMutation.isPending,
    isWithdrawing: withdrawMutation.isPending,
    isDeleting: deleteMutation.isPending,
  };
};
