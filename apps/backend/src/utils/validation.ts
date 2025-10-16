export class ValidationError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'ValidationError';
  }
}

export const validateHeatCycle = (
  duration: number,
  cycle: number
): void => {
  if (isNaN(duration) || duration <= 0) {
    throw new ValidationError('Invalid duration: must be a positive number');
  }
  if (isNaN(cycle) || cycle <= 0) {
    throw new ValidationError('Invalid cycle: must be a positive number');
  }
};

export const validateDeviceId = (deviceId: string | undefined): string => {
  if (!deviceId || deviceId.trim() === '') {
    throw new ValidationError('deviceId is required');
  }
  return deviceId.trim();
};

export const validateConnectionType = (
  type: string | undefined
): 'device' | 'frontend' => {
  if (type !== 'device' && type !== 'frontend') {
    return 'device'; // Default fallback
  }
  return type;
};
