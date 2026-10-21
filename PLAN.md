# RescueVision Edge — Flagship Robotics & Edge-AI Project Plan

**Version:** 1.0 · **Date:** 20 October 2026  
**Owner:** Independent engineering project  
**Status:** Concept / user discovery not yet validated  
**Planning assumption:** 8–12 focused hours/week over 9–12 months.

## 1. Executive summary

**Mission:** Build an offline-first, human-in-the-loop system that helps drone search teams review RGB/thermal aerial footage, identify candidate people, locate and revisit promising sightings, and produce auditable mission evidence.

**Engineering objective:** Demonstrate end-to-end competence in modern C++, real-time video pipelines, embedded inference, computer vision, tracking, sensor fusion, robotics middleware, simulation, mission planning, safety behavior, and testing.

**Commercial hypothesis (not validated):** Small emergency-response teams and professional drone operators may value faster or more reliable footage review with privacy-preserving offline processing, cross-platform import, actionable evidence records, and dependable recovery from missing connectivity. Competitors already exist; payability and differentiation must be validated before commercialization.

**First product:** A laptop-based postflight review and evidence-triage assistant. **Second:** edge deployment on Raspberry Pi 4. **Third:** supervised ROS 2/PX4 mission demonstration in simulation. Actual autonomous flight is excluded from the first year unless independently authorized and made safe.

## 2. Problem and user

**Primary persona:** Drone search operator reviewing long RGB or thermal recordings.  
**Secondary persona:** Incident coordinator reviewing unverified candidate sightings and search coverage.  
**Economic buyer (to investigate):** SAR organizations, response agencies, professional search/drone service operators. Public-sector buying cycles and reliability requirements could make this a difficult commercial market.

### Jobs to be done
1. Import footage reliably despite poor internet access.
2. Surface candidate sightings quickly, with clear evidence and uncertainty.
3. Avoid reviewing the same stationary candidate in every frame.
4. Link each sighting to a timestamp, video frame, model version, and, only if trustworthy, geographic coordinates.
5. Mark false alarms, prioritize rechecks, export a concise report.
6. Replay the exact session for debugging, audit, and training.

### Non-goals
- Face recognition or identifying people.
- Replacing trained drone pilots or rescue command.
- Autonomous live search or unpiloted mission decisions.
- Certifying that an area contains no people.
- Asserting geographic coordinates without synchronized, calibrated telemetry.
- Designing a custom airframe, radio stack, or flight controller.
- Multi-vehicle autonomy in the first release.

## 3. Scope and release strategy

### Release A: `v0.1` — Replay-to-report (MVP)
Input: local RGB/thermal image/video file.  
Processing: decoder -> normalized frames -> person detector -> tracking -> event deduplication -> indexed evidence.  
Operator output: list of reviewable detections, video timestamps, image thumbnails, confirm/reject controls, CSV/JSON report.  
All processing can run locally without a network connection. No drone hardware needed.

### Release B: `v0.2` — Edge and location
Deploy the same C++ inference/tracking core to Raspberry Pi 4 ARM64; profile CPU, RAM, thermals, and sustained throughput. Add an optional recorded telemetry adapter. Overlay flight path and show geographic estimates **only** with time synchronization, camera intrinsics/extrinsics, known altitude/attitude, and documented uncertainty. Otherwise show `location_unavailable`.

### Release C: `v0.3` — Simulated mission intelligence
Use Ubuntu 24.04 + ROS 2 Jazzy + Gazebo Harmonic + PX4 SITL. Consume simulated image + pose data, emit candidate sightings, propose follow-up waypoints and accept operator approval. PX4 owns stabilization and flight safety; start with existing PX4 position controllers. Implement loss-of-link and estimator-unavailable tests. This is a simulation-only demonstration.

### Release D: `v0.4` — Pilot evidence
Conduct operator workflow interviews, supervised offline evaluations and usability testing with authorized footage. Compare against manual review and existing tools; decide whether to advance, pivot to industrial inspection, or remain an open engineering portfolio.

## 4. Technical architecture

```text
          SOURCE ADAPTERS
   RGB / IR files | camera | Gazebo image
                    |
           Frame ingest + clock
                    |
       Preprocess / resize / tile
                    |
       ONNX Runtime C++ detector
                    |
      Tracker + temporal clustering
                    |
     Candidate event + uncertainty
             /             \
  Telemetry adapter         Evidence store
 (GPS/IMU/camera pose)       (SQLite + media)
             \             /
          Sighting/Geo engine
                    |
        Local operator application
      Review | map | report | replay
                    |
           Mission supervisor
   Human approval | failsafe checks
                    |
    ROS 2 bridge -> PX4 SITL / Gazebo
                 (simulation only)
```

### Components, boundaries, contracts
- `frame_source`: RGB/IR frame, timestamp, source ID, monotonically increasing sequence number; reports dropped/late frames.
- `detector`: returns image-space boxes, confidence, class, model ID, inference latency; never outputs a verified identity.
- `tracker`: generates temporary track IDs and candidate clusters; reset on scene cuts and long timestamp gaps.
- `telemetry`: timestamped pose, position, quality flags, calibration version and coordinate frame.
- `geolocator`: projects image detections only when camera model and synchronized pose are available; emits location plus covariance / error bounds or an explicit unavailable reason.
- `event_store`: immutable source references, model/config/version, candidate evidence, timestamps and operator decisions.
- `review_ui`: queue, video timeline, mission track, confidence/quality warnings, CSV/JSON exports.
- `mission_supervisor`: state machine `IDLE -> REVIEWING -> PROPOSED -> APPROVED -> SIMULATING -> COMPLETED/ABORTED`; rejects commands with stale telemetry, unknown flight state or no operator approval.

### Example evidence event

```json
{
  "event_id": "EVT-000184",
  "mission_id": "MISSION-0001",
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

Never invent geolocation. A pixel centroid is not a latitude/longitude. Enforce frame conventions (ROS ENU vs PX4 NED), time synchronization, altitude references and uncertainty handling explicitly.

## 5. Technology and hardware

| Layer | Starting choice | Notes |
|---|---|---|
| OS for robotics | Ubuntu 24.04 | Native Linux preferred for PX4/Gazebo; WSL2 can be used for early development but may require GUI/network adjustments |
| Robotics middleware | ROS 2 Jazzy | Current officially documented PX4 companion-stack path |
| Simulation / autopilot | Gazebo Harmonic + PX4 SITL | No custom flight controller initially |
| Core language | C++20 + CMake | Build from your ongoing C++/ONNX work |
| Video & image processing | OpenCV + FFmpeg as needed | RGB and grayscale/thermal video decoding |
| Inference | ONNX Runtime C++ | Pluggable backend; CPU baseline first |
| Training | Python + PyTorch / Ultralytics where suitable | Keep training separate from deployed C++ runtime |
| Data | SQLite + file storage, JSON exports | Offline-first / deterministic replay |
| Testing | GoogleTest, CTest, GitHub Actions | Unit, integration, regression and end-to-end tests |
| Edge | Existing Raspberry Pi 4 | Benchmark honestly; do not assume real-time performance |
| UI | Lightweight local web application or Qt | Choose after early workflow interviews |
| Mapping | MapLibre or equivalent | Offline basemap optional; only show grounded coordinates |
| Flight bridge | PX4 ROS 2 interface / DDS | Depend on version-compatible px4_msgs and interface |

**Licensing:** Ultralytics YOLO11 is offered under AGPL-3.0 and enterprise licensing; understand obligations before distribution or commercial embedding. Dataset licenses and privacy obligations must be reviewed separately. Do not assume a trained model or data can be commercialized solely because it is available online.

## 6. Dataset and model plan

1. Preserve the current VisDrone work for aerial RGB baseline and C++ deployment practice.
2. Establish thermal baseline with HIT-UAV (2,898 annotated frames) or a suitable SAR-specific dataset.
3. Evaluate on separated scenes, times, or missions to avoid leakage across nearly identical frames.
4. Explore AIResQ (2026): high-resolution thermal images, atypical body poses, plus real-flight benchmark. Verify availability, license and any use restrictions before incorporation.
5. Document image-space label formats and draw overlays to catch common coordinate conversion errors. A reported HIT-UAV GitHub issue identifies a possible center-vs-top-left conversion pitfall; verify independently.
6. Track per-subset metrics by altitude, person size, occlusion, time of day, and RGB vs IR source.

**Key metrics:** Recall and precision at selected thresholds; average precision; false alarms per recording hour; recall on small subjects; latency from frame arrival to detection; end-to-end operator review time; geolocation error when ground truth exists.

## 7. Milestones and definition of done

| Phase | Indicative duration | Deliverable | Exit gate |
|---|---:|---|---|
| 0. User discovery + benchmark | Weeks 1–2 | 5 operator conversations, 2 competitor workflows, baseline dataset | Clear pain, comparison and target decision; otherwise narrow/pivot |
| 1. Reliable C++ detection core | Weeks 3–6 | Video-to-detections CLI, versioned config, metrics | Repeatable benchmark; tests and documented failure cases |
| 2. Tracking + evidence review | Weeks 7–10 | De-duplicated sightings and local review UI | Exact replay + confirm/reject + reliable export |
| 3. Thermal improvement | Weeks 11–16 | Thermal model comparison | Better detection/trade-off than baseline on untouched holdout |
| 4. Pi 4 edge deployment | Weeks 17–20 | ARM64 build + profiling | Stable long-run test with documented limits |
| 5. Robotics simulation | Weeks 21–30 | ROS 2/PX4 simulated telemetry, waypoint revisit | Operator-approved simulation succeeds and fails safely |
| 6. Sensor fusion + localization | Weeks 31–38 | Timestamped camera pose + location quality flags | Bounded simulation error; explicit rejection of invalid data |
| 7. Validation & polish | Weeks 39–52 | Pilot protocol, technical report, demos, README | Evidence-backed go/no-go product decision |

Durations assume 8–12 focused hours/week and should change as actual blockers appear. Do not begin the robotics simulator until the core replay-to-report application is usable.

## 8. Testing and verification plan

### Test layers
- **Unit:** image coordinate transforms, preprocessing, box conversion, overlap suppression, timestamp comparisons, track association, state-machine transitions.
- **Data QA:** identical image-label stems, valid class IDs, image dimensions, no train/test scene overlap, spot-checked overlays.
- **Integration:** reproducible decode -> inference -> track -> save -> load -> UI review -> export.
- **Performance:** FPS, median and 95th-percentile total latency, CPU utilization, memory footprint, edge CPU temperature and throttling.
- **Robotics:** pose/frame conversion, telemetry staleness, clock drift, heartbeat loss, estimator loss, simulator termination, no-approval guard.
- **Usability:** operator task completion time, correct identification rate, review burden, log clarity.

### Proposed acceptance targets (engineering goals, not validated field claims)
- Detection results reproducible on a version-pinned holdout dataset.
- 100% of accepted or rejected evidence events can be traced back to a source frame and model version.
- Zero coordinates displayed as valid when pose/calibration prerequisites are absent in automated tests.
- No revisit command issued without explicit human approval in 100/100 automated simulation tests.
- No crash or silent data corruption in a 60-minute recorded-video stress test.
- Quantify false alarms per hour and person-level missed detections; do not call the system rescue-ready based on mAP alone.
- Edge performance target is negotiated after Raspberry Pi 4 profiling, not invented in advance.

### Required failure injections
A corrupted frame; video timestamp jumps; low-contrast humans; people at image edge; thermal false hotspots; moving animals; ground-truth negatives; lost GPS; bad compass; stale image/pose; network disconnect; sudden camera change; incorrect ENU/NED conversion; detector stalls and high CPU load.

## 9. Real-world validation

### Discovery, before building a complex product
Speak with ~5–8 drone operators, rescue volunteers, instructors, or industrial inspection service providers. Do not presume they are buyers.

Ask them to show their real workflow where confidentiality permits:
- How do you review a 20–40 minute mission today?
- What gets missed or repeated? What evidence records are required?
- Which systems/software do you already use? What costs time?
- What is worse: 10 extra alerts or one missed candidate?
- Which video formats, cameras and telemetry exports are essential?
- Would data need to stay offline? What security and procurement constraints apply?
- What result would justify an independent trial? Who could approve one?

### Evidence-based A/B test
Select footage with consent and independently established labels. Compare `manual review` vs `RescueVision-assisted review`, counterbalancing review order and using multiple operators if possible. Measure completion time, candidates found, misses, false alarms, trust, and report usefulness. Never claim operational search benefit solely from a test dataset benchmark.

### Go / no-go decision
- **Continue as a product** if multiple users show the same pain, a defensible measurable improvement emerges, and an organization is willing to trial or pay.
- **Continue as a technical portfolio** if the engineering is strong but buyer access or trust requirements are prohibitive.
- **Pivot wedge to industrial inspection** if operators demonstrate clearer commercial demand for similar offline footage review + evidence mapping.

## 10. Commercial strategy

**Initial offer:** Offline, human-in-the-loop mission-review software for team use; no drone manufacturer lock-in in the prototype.

**Positioning to investigate:** Faster evidence review, portability, transparent model limitations, traceable findings, local data control, and integration with existing drone exports. Each claim must be demonstrated against existing solutions such as ADIAT, DJI, Robotto, and other operators' actual tools.

**Commercial obstacles:** Public procurement, lengthy trust-building, scarce labeled real-mission data, privacy/security, integration effort, safety validation, very small buyer populations, and sophisticated existing alternatives. Avoid pricing assumptions until interviews.

**Potential revenue experiments:** Paid evaluation / setup for drone operators, enterprise licenses for validated workflows, custom integration or maintenance contracts. Pricing is not established.

## 11. Safety, privacy, and legal boundary

- No unsupervised live flight by this system in the MVP.
- Under EASA rules, fully autonomous operations are not allowed in the EU open category. Real operations must comply with applicable aviation rules, permissions, and operator requirements.
- Preserve operator override, fail-safe defaults, and explicit separation between suggestions and flight commands.
- Do not assert a person is absent due to a negative model inference.
- Seek consent/legal authority for real-world imagery; minimize personal data, enforce retention and access controls for sensitive recordings.
- Do not leak drone IP/MAC details or record identifiers in publicly posted logs.
- A toy drone with unstable flight is not an appropriate validation platform for mission-critical autonomy.

## 12. Repository layout

```text
rescuevision-edge/
├── README.md
├── PLAN.md
├── LICENSE
├── CMakeLists.txt
├── cpp/
│   ├── core/           # timestamps, config, event models
│   ├── video/          # video/camera adapters
│   ├── inference/      # ONNX detector backends
│   ├── tracking/       # track IDs, deduplication
│   ├── localization/   # calibration/pose/coordinate logic
│   └── apps/           # replay CLI / local API
├── ros2_ws/src/
│   ├── vision_bridge/
│   ├── mission_supervisor/
│   └── telemetry_adapter/
├── ml/
│   ├── data_tools/
│   ├── experiments/
│   └── export/
├── ui/
├── configs/
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── simulation/
│   └── performance/
├── datasets/README.md  # download instructions, not private/raw datasets
├── scripts/
└── docs/
    ├── architecture.md
    ├── metrics.md
    ├── safety.md
    ├── customer-discovery.md
    └── validation-report.md
```

Pin software versions and artifact hashes. Store larger datasets and recordings outside Git and never commit private source footage or personal identifiers.

## 13. First 14-day sprint

**Day 1:** Finish or stabilize your existing VisDrone ONNX export/C++ inference; take a reproducible baseline.  
**Day 2:** Create `rescuevision-edge` repository and docs; define event schema and command line.  
**Day 3–4:** Implement video decoding, timestamps and file adapter.  
**Day 5:** ONNX detection on recorded frames with logging; save results to JSON.  
**Day 6–7:** Build visualization overlays, inspect 20 tricky cases, record throughput.  
**Day 8–9:** Add track IDs / elementary deduplication, evidence thumbnails.  
**Day 10:** Add automated tests for box, timestamp and JSON transforms.  
**Day 11:** Test on 2–3 different prerecorded videos; document errors.  
**Day 12:** Compare manually reviewed candidate counts with model output.  
**Day 13–14:** Conduct initial operator outreach; compile unanswered product questions and decide the next sprint.

**Definition of done, sprint 1:** A reproducible CLI command reads a video, emits reviewable candidates with timestamps and frame thumbnails, and writes a machine-readable report. Any geolocation remains explicitly unavailable unless trustworthy telemetry has been integrated.

## 14. Recommended study focus

- **Modern C++:** RAII, const correctness, smart pointers, concurrency, profiling, deterministic tests.
- **Computer vision:** precision/recall calibration, scale sensitivity, tracking association, thermal/RGB domain shift.
- **Robotics:** ROS 2 messages/QoS, PX4 flight states and fail-safes, Gazebo SITL.
- **State estimation:** camera calibration, coordinate frames, synchronized telemetry, uncertainty.
- **Control:** PID and trajectory following in simulation, loop stability and disturbances.
- **Systems:** serialization, data provenance, offline operation, fault recovery.

## 15. Reference links (checked October 2026)

- Robotto perception/autonomy platform: https://robotto.ai/
- Texas SAR ADIAT 2.1 (existing competing capability): https://www.texsar.org/automated-drone-image-analysis-tool/
- DJI SAR offering: https://enterprise.dji.com/public-safety/search-and-rescue
- AIResQ paper: https://www.nature.com/articles/s41597-026-07663-9
- AIResQ benchmark reference: https://zenodo.org/records/17405074
- AIResQ dataset reference: https://zenodo.org/records/19677785
- HIT-UAV dataset: https://github.com/suojiashun/HIT-UAV-Infrared-Thermal-Dataset
- PX4 + ROS 2 official guide: https://docs.px4.io/main/en/ros2/user_guide
- PX4 Offboard / fail-safes: https://docs.px4.io/main/en/flight_modes/offboard
- EASA autonomous vs automatic: https://www.easa.europa.eu/en/faq/116449
- Ultralytics YOLO11 / licensing: https://docs.ultralytics.com/models/yolo11

---

**Success criterion for the year:** Not merely “a model detects people.” It is a reliable, measurable, documented, modular system that integrates AI, C++, edge hardware and supervised robotics—and a clear, evidence-based decision about whether anyone needs it enough to deploy or purchase it.
