import { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Accordion, AccordionContent, AccordionItem, AccordionTrigger } from '@/components/ui/accordion';
import { motion } from 'framer-motion';
import { AlertCircle, Loader } from 'lucide-react';
import { SessionGroup, ApiResponse } from '../../../types';

const SessionTracker = () => {
  const [sessions, setSessions] = useState<SessionGroup[]>([]);
  const [totalConsumption, setTotalConsumption] = useState<string>('0');
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<Error | null>(null);

  useEffect(() => {
    const fetchSessions = async () => {
      try {
        setLoading(true);
        setError(null);
        const response = await fetch('/api/sessions');
        if (!response.ok) {
          throw new Error('Failed to fetch sessions');
        }
        const data: ApiResponse = await response.json();
        setSessions(data.sessions);
        setTotalConsumption(data.totalConsumption);
      } catch (err) {
        if (err instanceof Error) {
          setError(err);
        } else {
          setError(new Error('An unknown error occurred'));
        }
        console.error(err);
      } finally {
        setLoading(false);
      }
    };

    fetchSessions();
  }, []);

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <Loader className="h-8 w-8 animate-spin text-primary" />
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex flex-col items-center justify-center h-64 text-destructive">
        <AlertCircle className="h-8 w-8 mb-2" />
        <p>Error loading sessions: {error.message}. Please try again later.</p>
      </div>
    );
  }

  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.5 }}
    >
      <Card className="w-full max-w-2xl mx-auto">
        <CardHeader>
          <CardTitle className="flex justify-between items-center">
            <span>Heizbox Sessions</span>
            <span className="text-lg font-mono bg-muted text-muted-foreground px-3 py-1 rounded-md">
              {totalConsumption}g
            </span>
          </CardTitle>
        </CardHeader>
        <CardContent>
          {sessions.length > 0 ? (
            <Accordion type="single" collapsible className="w-full">
              {sessions.map((session, index) => (
                <AccordionItem value={`item-${index}`} key={index}>
                  <AccordionTrigger>
                    <div className="flex justify-between w-full pr-4">
                      <span>Session {sessions.length - index}</span>
                      <span className="text-sm text-muted-foreground">{session.length} Clicks</span>
                    </div>
                  </AccordionTrigger>
                  <AccordionContent>
                    <div className="space-y-2">
                      {session.map((click) => (
                        <div key={click.id} className="flex justify-between text-sm font-mono text-muted-foreground">
                          <span>{new Date(click.created_at).toLocaleTimeString('de-DE')}</span>
                          <span>{click.duration}s</span>
                        </div>
                      ))}
                    </div>
                  </AccordionContent>
                </AccordionItem>
              ))}
            </Accordion>
          ) : (
            <p className="text-center text-muted-foreground py-8">No sessions recorded for today.</p>
          )}
        </CardContent>
      </Card>
    </motion.div>
  );
};

export default SessionTracker;