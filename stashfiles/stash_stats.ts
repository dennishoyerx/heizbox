// File: apps/frontend/src/app/stash/StashStatsPage.tsx
import { useState } from 'react';
import { useQuery } from '@tanstack/react-query';
import { Flex, Text, Button, SegmentedControl } from '@radix-ui/themes';
import { BarChart, Bar, LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid, PieChart, Pie, Cell } from 'recharts';
import { fetchStashStats } from '../../api-stash';
import { useTheme } from '../hooks/useTheme';

function StashStatsPage() {
  const { theme } = useTheme();
  const [range, setRange] = useState('30d');
  
  const { data, isLoading, isError, error } = useQuery({
    queryKey: ['stashStats', range],
    queryFn: () => fetchStashStats(range),
  });

  const axisStrokeColor = 'var(--gray-11)';
  const tooltipBgColor = 'var(--color-panel-solid)';
  const gridColor = 'var(--gray-6)';
  const COLORS = [
    'var(--blue-9)',
    'var(--green-9)',
    'var(--amber-9)',
    'var(--orange-9)',
    'var(--purple-9)',
  ];

  if (isLoading) {
    return (
      <Flex direction="column" gap="3" maxWidth="1200px" className="mx-auto p-4">
        <Text>Lade Statistiken...</Text>
      </Flex>
    );
  }

  if (isError) {
    return (
      <Flex direction="column" gap="3" maxWidth="1200px" className="mx-auto p-4">
        <Text color="red">Fehler: {error?.message}</Text>
      </Flex>
    );
  }

  // Datenaufbereitung für Charts
  const itemDistribution = data?.items.map(item => ({
    name: item.item_name,
    value: item.quantity_current,
  })) || [];

  const withdrawalTrend = data?.daily_withdrawals?.map(day => ({
    date: day.date,
    quantity: day.total_quantity,
    count: day.withdrawal_count,
  })) || [];

  const itemConsumption = data?.items.map(item => ({
    name: item.item_name,
    consumed: item.total_withdrawn,
    remaining: item.quantity_current,
  })) || [];

  return (
    <Flex direction="column" gap="4" maxWidth="1200px" className="mx-auto p-4">
      <Flex justify="between" align="center">
        <Text size="6" weight="bold">Stash Statistiken</Text>
        <SegmentedControl.Root value={range} onValueChange={setRange}>
          <SegmentedControl.Item value="7d">7 Tage</SegmentedControl.Item>
          <SegmentedControl.Item value="30d">30 Tage</SegmentedControl.Item>
          <SegmentedControl.Item value="90d">90 Tage</SegmentedControl.Item>
        </SegmentedControl.Root>
      </Flex>

      {/* Übersichtskarten */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <div className="bg-gradient-to-br from-blue-500 to-blue-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Gesamt Items</h3>
          <p className="text-3xl font-bold mt-2">{data?.total_items || 0}</p>
        </div>

        <div className="bg-gradient-to-br from-green-500 to-green-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Verfügbar</h3>
          <p className="text-3xl font-bold mt-2">{data?.total_quantity_current.toFixed(1) || 0}g</p>
        </div>

        <div className="bg-gradient-to-br from-orange-500 to-orange-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Entnommen</h3>
          <p className="text-3xl font-bold mt-2">{data?.total_quantity_withdrawn.toFixed(1) || 0}g</p>
        </div>
      </div>

      {/* Charts */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Bestandsverteilung */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <Text size="5" weight="bold" className="mb-4">Aktuelle Bestandsverteilung</Text>
          <ResponsiveContainer width="100%" height={300}>
            <PieChart>
              <Pie
                data={itemDistribution}
                cx="50%"
                cy="50%"
                labelLine={false}
                label={(entry) => `${entry.name}: ${entry.value.toFixed(1)}g`}
                outerRadius={100}
                fill="#8884d8"
                dataKey="value"
              >
                {itemDistribution.map((entry, index) => (
                  <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                ))}
              </Pie>
              <Tooltip />
            </PieChart>
          </ResponsiveContainer>
        </div>

        {/* Entnahme-Trend */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <Text size="5" weight="bold" className="mb-4">Entnahme-Verlauf</Text>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={withdrawalTrend}>
              <CartesianGrid strokeDasharray="3 3" stroke={gridColor} />
              <XAxis dataKey="date" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
              />
              <Line 
                type="monotone" 
                dataKey="quantity" 
                stroke="#82ca9d" 
                name="Menge (g)"
                strokeWidth={2}
              />
            </LineChart>
          </ResponsiveContainer>
        </div>

        {/* Verbrauch pro Item */}
        <div className="bg-slate-800 p-6 rounded-lg shadow lg:col-span-2">
          <Text size="5" weight="bold" className="mb-4">Verbrauch pro Item</Text>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={itemConsumption}>
              <CartesianGrid strokeDasharray="3 3" stroke={gridColor} />
              <XAxis dataKey="name" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
              />
              <Bar dataKey="consumed" fill="#ff7300" name="Entnommen (g)" />
              <Bar dataKey="remaining" fill="#82ca9d" name="Verbleibend (g)" />
            </BarChart>
          </ResponsiveContainer>
        </div>
      </div>
    </Flex>
  );
}

export default StashStatsPage;