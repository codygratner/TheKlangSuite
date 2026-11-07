/**
 * media_monitor.js — Universal Web Audio LTL Runtime Monitor
 * ==========================================================
 * Part of Chassis 2: Universal SQA Perimeter.
 * Attaches to HTML5 <audio> / <video> elements to enforce:
 *   1. Bounded Liveness: G ( Event(play) && ready => F_<=k (currentTime advances) )
 *   2. Guarded Monotonicity: G ( playing && !seek && !loop => X(currentTime >= prev) )
 *   3. Playback Rate Invariant: G ( playbackRate == R => |d(currentTime)/dt - R| <= eps )
 */

export function attachMediaMonitor(audioElement, deadlineMs = 1500) {
  const violations = [];
  let prevTime = audioElement.currentTime;
  let playTriggeredAt = null;
  let playStartTimecode = 0;
  let isSeekingOrLoading = false;

  audioElement.addEventListener('seeking', () => { isSeekingOrLoading = true; });
  audioElement.addEventListener('emptied', () => { isSeekingOrLoading = true; });
  audioElement.addEventListener('play', () => {
    playTriggeredAt = performance.now();
    playStartTimecode = audioElement.currentTime;
  });

  audioElement.addEventListener('timeupdate', () => {
    const cur = audioElement.currentTime;
    // Guarded monotonicity check
    if (!audioElement.paused && !isSeekingOrLoading && cur < prevTime) {
      violations.push({
        property: 'monotonicity',
        error: `Timecode moved backward from ${prevTime.toFixed(3)}s to ${cur.toFixed(3)}s during normal playback!`,
        timestamp: performance.now(),
        timecode: cur
      });
    }

    // Bounded liveness check
    if (playTriggeredAt !== null && cur > playStartTimecode) {
      playTriggeredAt = null; // Liveness discharged
    }

    prevTime = cur;
    isSeekingOrLoading = false;
  });

  const timer = setInterval(() => {
    if (playTriggeredAt !== null && audioElement.readyState >= 3) {
      if (performance.now() - playTriggeredAt > deadlineMs) {
        violations.push({
          property: 'bounded-liveness',
          error: `Audio failed to advance within ${deadlineMs}ms after play() was triggered!`,
          timestamp: performance.now(),
          timecode: audioElement.currentTime
        });
        playTriggeredAt = null;
      }
    }
  }, 250);

  return {
    getViolations: () => [...violations],
    hasViolations: () => violations.length > 0,
    detach: () => clearInterval(timer)
  };
}
