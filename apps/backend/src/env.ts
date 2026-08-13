import { db } from "./db.js";
import { DeviceStatusHub, DeviceStatusNamespace } from "./hub.js";

export interface Runtime {
  env: Env;
  hub: DeviceStatusHub;
}

export function createRuntime(): Runtime {
  const hub = new DeviceStatusHub(db);
  const env = {
    db,
    DEVICE_STATUS: new DeviceStatusNamespace(hub) as unknown as DurableObjectNamespace,
    CF_VERSION_METADATA: { id: "local" },
  } as Env;
  return { env, hub };
}
