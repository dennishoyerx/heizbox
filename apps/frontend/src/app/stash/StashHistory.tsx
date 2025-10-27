import { useEffect, useState } from 'react';
import { fetchRecentWithdrawals } from '@/api';

export function StashHistory() {
  const [entries, setEntries] = useState<{ name: string; amount: number; withdrawn_at: string }[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetchRecentWithdrawals()
      .then(setEntries)
      .catch(() => setError('Could not load withdrawal history.'));
  }, []);

  return (
    <div className="mt-6">
      <h2 className="text-lg font-semibold mb-2">Entnahmen (letzte 30 Tage)</h2>
      {error && <p className="text-red-500">{error}</p>}
      <div className="overflow-x-auto">
        <table className="w-full text-sm">
          <thead>
            <tr className="text-left">
              <th className="p-2">Menge</th>
              <th className="p-2">Name</th>
              <th className="p-2">Datum</th>
            </tr>
          </thead>
          <tbody>
            {entries.map((e, i) => (
              <tr key={`${e.withdrawn_at}-${e.name}-${i}`} className="border-t border-gray-200">
                <td className="p-2">{e.amount}g</td>
                <td className="p-2">{e.name}</td>
                <td className="p-2">{new Date(e.withdrawn_at).toLocaleDateString('de-DE', { day: '2-digit', month: '2-digit' })}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}
