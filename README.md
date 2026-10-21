# RescueVision Edge

**Offline-first, human-in-the-loop review of aerial search footage.**

RescueVision Edge helps drone search teams review RGB and thermal footage faster. It flags candidate people, groups repeated sightings into single reviewable events, and produces an auditable evidence report. Everything runs locally, with no internet connection required.

> **Status: pre-alpha / concept.** Nothing here is ready for operational use. User discovery is in progress and the core pipeline is under construction. See [Roadmap](#roadmap).

## Current implementation status

RescueVision Edge is in the initial engineering phase. The repository structure has been established, and development of the C++ perception engine is beginning.

**Existing technical foundation:** [VisDrone C++ Object Detection](https://github.com/justusa11/Visdrone)

The predecessor project includes:
- YOLO11n fine-tuning on aerial imagery.
- ONNX export and inference using C++17, OpenCV and ONNX Runtime.
- Custom image preprocessing, bounding-box decoding and non-maximum suppression.
- Python/C++ detection parity validation.

This implementation is being audited and will serve as the initial reference for RescueVision's modular C++20 inference engine.

**Not yet implemented in RescueVision:** Video ingestion, object tracking, evidence persistence, thermal detection, embedded deployment, ROS 2 integration and simulated mission control.

## Phase 01 — C++ Perception Engine

The first engineering milestone is a reproducible offline pipeline that processes recorded video and outputs structured detections.

- [x] Establish project repository structure.
- [ ] Configure C++20 and CMake build targets.
- [ ] Implement frame ingestion with timestamp handling.
- [ ] Integrate and validate ONNX Runtime inference.
- [ ] Export structured detection records.
- [ ] Add automated unit and integration tests.
- [ ] Benchmark throughput, latency and memory usage.
- [ ] Document failure cases and complete the engineering review.

The phase is complete only when the detection pipeline passes its defined tests and reproducibility requirements.

See [PLAN.md](PLAN.md) for the full development roadmap.

---

## Why

Search teams often fly 20–40 minute missions and then review the recordings by hand. That review is slow, repetitive, and easy to get wrong when tired. Connectivity in the field is often poor, and footage of people is sensitive.

RescueVision Edge aims to:

- **Surface candidates quickly**, with clear evidence and stated uncertainty.
- **Cut repeated review** by grouping the same stationary candidate across frames.
- **Keep every finding traceable** to a source frame, timestamp and model version.
- **Work offline**, so footage never has to leave the team's machine.
- **Replay any session exactly** for audit, debugging and training.

## What it is not

- Not face recognition, and it never identifies who a person is.
- Not a replacement for trained pilots or rescue command. Every candidate is reviewed by a human.
- Not proof that an area is empty. A negative result means "nothing flagged", never "no one there".
- Not a source of invented coordinates. A location is shown only when synchronized, calibrated telemetry supports it. Otherwise it is marked `location_unavailable`.
- Not autonomous flight. Any robotics work is simulation-only and requires operator approval.

## How it works

```text
 RGB / thermal video ──► frame ingest + clock ──► preprocess / tile
                                                        │
                                              ONNX Runtime detector
                                                        │
                                          tracker + temporal clustering
                                                        │
                                       candidate events + uncertainty
                                          │                       │
                              telemetry (optional)         evidence store
                                          └──► geolocation ◄──────┘
                                                        │
                                       local review: confirm / reject / export
```

Each candidate becomes an evidence event:

```json
{
  "event_id": "EVT-000184",
  "source_id": "thermal-01",
  "timestamp_utc": "2026-10-20T12:00:00.000Z",
  "frame_index": 9134,
  "class": "candidate_person",
  "confidence": 0.72,
  "bounding_box_xyxy": [316, 122, 334, 155],
  "track_id": "T-017",
  "model_version": "sar-detector-v1",
  "location_status": "unavailable_missing_calibration",
  "review_status": "unverified"
}
```

## Roadmap

| Release | Scope | Status |
|---|---|---|
| **v0.1** Replay-to-report | Local video → detections → tracking → deduplicated candidates → review → CSV/JSON report | In progress |
| **v0.2** Edge + location | Same C++ core on Raspberry Pi 4 (ARM64); optional recorded telemetry; geolocation only with valid calibration | Planned |
| **v0.3** Simulated mission | ROS 2 Jazzy + Gazebo Harmonic + PX4 SITL; operator-approved revisit waypoints; failsafe tests | Planned |
| **v0.4** Pilot evidence | Operator usability tests; assisted vs. manual review comparison on consented footage | Planned |

Product decisions at each stage are driven by operator interviews. See `docs/customer-discovery.md`.

## Technology

| Layer | Choice |
|---|---|
| Core | C++20, CMake |
| Video | OpenCV, FFmpeg |
| Inference | ONNX Runtime (C++), CPU baseline first |
| Training | Python / PyTorch (kept separate from the deployed runtime) |
| Storage | SQLite + files, JSON/CSV export |
| Testing | GoogleTest, CTest, GitHub Actions |
| Edge | Raspberry Pi 4 (ARM64) |
| Robotics (v0.3) | Ubuntu 24.04, ROS 2 Jazzy, Gazebo Harmonic, PX4 SITL |

## Getting started

Build instructions will be added with v0.1. Planned requirements:

- Ubuntu 24.04 (recommended) or another Linux with a C++20 compiler
- CMake ≥ 3.24
- OpenCV, ONNX Runtime, GoogleTest

## Repository layout

```text
cpp/        core, video, inference, tracking, localization, apps
ros2_ws/    ROS 2 packages (simulation only)
ml/         data tools, experiments, model export
ui/         local review application
configs/    versioned pipeline configs
tests/      unit, integration, simulation, performance
docs/       architecture, metrics, safety, discovery, validation
```

## Measuring quality

Accuracy scores alone do not make a search tool trustworthy. The project tracks:

- Recall and precision at chosen thresholds, especially on small and partly hidden people
- False alarms per hour of footage
- Latency from frame arrival to detection
- Operator review time compared with manual review
- Geolocation error, where ground truth exists

Results are reported per altitude, subject size, time of day and RGB vs. thermal source, on held-out scenes that never overlap with training data.

## Safety and privacy

- A human reviews every candidate. The system suggests and never decides.
- No live autonomous flight. Real operations must follow EASA and local aviation rules.
- Footage of people is personal data and is handled under GDPR. Use real-world imagery only with consent or legal authority.
- Raw footage, private datasets and personal identifiers are never committed to this repository.

## Data and model licensing

Dataset and model licenses are checked before use in anything distributed. Several aerial and thermal datasets are restricted to research use, and some detector frameworks (e.g. Ultralytics YOLO, AGPL-3.0) carry obligations for commercial distribution. See `datasets/README.md`.

## License

To be decided. All rights reserved until a license is chosen.

## Contact

Justus (Jay) Anoruo · [github.com/justusa11](https://github.com/justusa11)