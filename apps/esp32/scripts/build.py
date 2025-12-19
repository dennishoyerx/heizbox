Import("env")
import multiprocessing
import platform

# CPU-Kerne ermitteln
cpu_count = multiprocessing.cpu_count()

# Auf Windows: Nicht alle Kerne nutzen (instabil)
if platform.system() == "Windows":
    # Nutze max. 75% der Kerne auf Windows
    jobs = max(1, int(cpu_count * 0.75))
else:
    jobs = cpu_count

env.SetOption('num_jobs', jobs)
print(f"Building with {jobs} parallel jobs (CPU cores: {cpu_count})")

# Windows: Long path support
if platform.system() == "Windows":
    import os
    os.environ['PLATFORMIO_BUILD_FLAGS'] = '-D LONG_PATH_SUPPORT'
    print("Windows long path support enabled")