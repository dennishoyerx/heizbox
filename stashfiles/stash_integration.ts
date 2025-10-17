// File: apps/frontend/src/app/stash/index.ts
export { default as StashPage } from './StashPage';
export { default as StashStatsPage } from './StashStatsPage';
export { AddStashItemDialog } from './AddStashItemDialog';
export { WithdrawDialog } from './WithdrawDialog';
export { useStash } from './useStash';

// ============================================
// Routing-Update in apps/frontend/src/app/app.tsx
// ============================================
// Import hinzufügen:
// import { StashPage, StashStatsPage } from "./stash";

// Routes erweitern:
// <Routes>
//   <Route path="/" element={<SessionPage />} />
//   <Route path="/usage" element={<UsagePage />} />
//   <Route path="/stash" element={<StashPage />} />
//   <Route path="/stash/stats" element={<StashStatsPage />} />
// </Routes>

// ============================================
// Header-Update für Navigation
// File: apps/frontend/src/app/components/Header.tsx (erweitert)
// ============================================
// In der DropdownMenu.Content nach UsageItem:
/*
<DropdownMenu.Item asChild>
  <Link to="/stash">Stash</Link>
</DropdownMenu.Item>

<DropdownMenu.Item asChild>
  <Link to="/stash/stats">Stash Statistiken</Link>
</DropdownMenu.Item>
*/

// ============================================
// WebSocket Type Update
// File: libs/types/src/index.ts
// ============================================
// Export hinzufügen:
// export * from './stash';

// Server WebSocket Message Type erweitern:
// import type { StashUpdatedMessage } from './stash';
// 
// export type ServerWebSocketMessage =
//   | DeviceStatusUpdate
//   | SessionDataMessage
//   | SessionCreatedMessage
//   | HeatCycleCompletedMessage
//   | StashUpdatedMessage;