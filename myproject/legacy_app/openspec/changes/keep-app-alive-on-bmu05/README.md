# keep-app-alive-on-bmu05

Keep the app running on BMU05: make app_entry the single launcher for both boot and RAUC post-install (with liveness-aware stale-PID cleanup), use a process-name-precise syshand liveness check, and gate the CCU comm-fail system reboot behind an 'ever communicated' latch.
