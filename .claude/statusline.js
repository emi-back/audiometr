#!/usr/bin/env node
/*
 * Claude Code status line for the audiometr project.
 * Shows: context window usage, and background/async agents currently working.
 *
 * Input: JSON payload piped on stdin by Claude Code (see statusLine docs).
 * Fields actually used:
 *   - context_window.used_percentage
 *   - transcript_path
 *
 * "Agents currently working" is NOT part of the stdin payload - there is no
 * live field for it. As a best-effort real signal, this script reads the
 * session transcript (transcript_path, a JSONL file Claude Code already
 * writes) and looks for two things Claude Code itself records there:
 *   1. "system" / "turn_duration" events, which include a
 *      "pendingBackgroundAgentCount" field - the most authoritative count
 *      available, but only refreshed at the end of each turn.
 *   2. tool_result entries for the "Agent" tool with
 *      toolUseResult.status === "async_launched", which carry an agentId
 *      and a human-readable description - used to label the most recently
 *      launched agents.
 * Limitations: the transcript has no explicit "agent completed" event that
 * this script recognizes, so agent names are a best-effort guess (the N
 * most recently launched agents, where N = pendingBackgroundAgentCount) and
 * can be stale or wrong. If pendingBackgroundAgentCount is unavailable
 * (e.g. very first turn, or older Claude Code versions without this field),
 * the script falls back to listing all async-launched agents found and
 * labels them "launched" instead of "running" to avoid overclaiming.
 */

'use strict';
const fs = require('fs');

function readStdin() {
  try {
    return fs.readFileSync(0, 'utf8');
  } catch (e) {
    return '';
  }
}

function safeParse(str) {
  try {
    return JSON.parse(str);
  } catch (e) {
    return null;
  }
}

const input = safeParse(readStdin()) || {};

// --- Context window usage ---
const cw = input.context_window || {};
let ctxStr = 'Ctx: --';
if (typeof cw.used_percentage === 'number') {
  ctxStr = 'Ctx: ' + Math.round(cw.used_percentage) + '% used';
}

// --- Agents currently working (best-effort, see header comment) ---
let agentsStr = 'Agents: none';

try {
  const transcriptPath = input.transcript_path;
  if (transcriptPath && fs.existsSync(transcriptPath)) {
    const lines = fs.readFileSync(transcriptPath, 'utf8').split('\n');

    let pendingCount = null;
    const launched = new Map(); // agentId -> { name, idx }

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i];
      if (!line) continue;
      const entry = safeParse(line);
      if (!entry) continue;

      if (
        entry.type === 'system' &&
        entry.subtype === 'turn_duration' &&
        typeof entry.pendingBackgroundAgentCount === 'number'
      ) {
        pendingCount = entry.pendingBackgroundAgentCount;
      }

      const tur = entry.toolUseResult;
      if (tur && tur.status === 'async_launched' && tur.agentId) {
        launched.set(tur.agentId, {
          name: tur.description || 'agent',
          idx: i,
        });
      }
    }

    const byRecency = Array.from(launched.values()).sort((a, b) => b.idx - a.idx);

    if (pendingCount !== null) {
      if (pendingCount > 0) {
        const names = byRecency.slice(0, pendingCount).map((a) => a.name);
        agentsStr = names.length
          ? `Agents: ${pendingCount} running (${names.join(', ')})`
          : `Agents: ${pendingCount} running`;
      }
    } else if (byRecency.length > 0) {
      const names = byRecency.map((a) => a.name);
      agentsStr = `Agents: ${byRecency.length} launched (${names.join(', ')})`;
    }
  }
} catch (e) {
  // Never let a status line crash the prompt - fall back to the default.
}

process.stdout.write(`${ctxStr} | ${agentsStr}`);
