import SessionTracker from './features/heizbox/SessionTracker';
import { ModeToggle } from './components/mode-toggle';

function App() {
  return (
    <div className="container mx-auto p-4 bg-red-500">
      <header className="flex justify-between items-center mb-8">
        <h1 className="text-3xl font-bold">Heizbox Web</h1>
        <ModeToggle />
      </header>
      <main>
        <SessionTracker />
      </main>
    </div>
  );
}

export default App;