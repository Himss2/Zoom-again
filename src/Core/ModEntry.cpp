// --- TEMPORARY: remove once TouchController exists (Step 2) ---
        // CORRECTED direction: on-device testing showed factor=1.7 looks
        // WIDE (zoomed out), not zoomed in - the value is a MULTIPLIER on
        // base FOV, not a divisor as first assumed. Smaller = zoomed in.
        // This also explains the old single-file project's "drag down
        // zooms in" bug: the drag mechanics weren't backwards, the
        // factor's meaning was.
        zoom_controller::BeginZoom();
        zoom_controller::UpdateDrag(-0.5f); // expect a visible, fixed zoom-in (factor -> 0.5)
        log.info("Core: test zoom engaged (factor should read ~0.50) - "
                 "confirm visually, then remove this block for Step 2");
        // --- end temporary block ---
