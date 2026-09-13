# GitHub-Only Compile Guide

You do not need to install a compiler on your laptop.

## Step 1 — Create the repository

1. Sign in to GitHub.
2. Click the **+** button in the top-right.
3. Click **New repository**.
4. Repository name: `AstraPDF`
5. Choose **Private** or **Public**.
6. Do not add a README, `.gitignore`, or license because this project already contains its own files.
7. Click **Create repository**.

## Step 2 — Upload the project

1. On the empty repository page click **uploading an existing file**.
2. Extract `AstraPDF-GitHub-Ready.zip` on your computer.
3. Open the extracted `AstraPDF` folder.
4. Drag **the contents inside that folder** into GitHub.

The repository root must contain:

```text
CMakeLists.txt
README.md
src/
docs/
installer/
.github/
```

Do not upload one extra outer `AstraPDF` directory above these files.

5. In the commit box type: `Initial AstraPDF source`
6. Click **Commit changes**.

## Step 3 — Open Actions

1. Click the repository **Actions** tab.
2. GitHub may show a message asking whether workflows should be enabled. Enable them.
3. Select **Build Windows EXE** in the left sidebar.
4. Click **Run workflow**.
5. Select the `main` branch.
6. Click the green **Run workflow** button.

A push to `main` also triggers the same build automatically.

## Step 4 — Read the build

Open the newest workflow run.

The job should execute these stages:

```text
Checkout
→ Install Qt 6.7.3
→ Configure CMake
→ Build Release
→ Run windeployqt
→ Verify AstraPDF.exe
→ Create portable ZIP
→ Install Inno Setup
→ Build Windows installer
→ Upload artifacts
```

If every step is green, the build succeeded.

If one step is red, click the red step and copy its log. The error log identifies the exact dependency/compiler/source failure instead of guessing.

## Step 5 — Download the finished application

At the bottom of the successful workflow run, GitHub shows **Artifacts**.

There will be two useful artifacts:

### AstraPDF-Portable-Windows-x64
Contains the portable program and Qt DLLs. Extract it and run `AstraPDF.exe`.

### AstraPDF-Setup-Windows-x64
Contains `AstraPDF-Setup-Windows-x64.exe`.

Download this one if you want normal Windows installation.

## Step 6 — Install on Windows 11

1. Download the installer artifact.
2. Extract GitHub's artifact ZIP.
3. Run `AstraPDF-Setup-Windows-x64.exe`.
4. Choose the install folder or keep the default.
5. Optionally create a desktop shortcut.
6. Finish installation.
7. Launch AstraPDF from the Start menu.

## Important

GitHub Actions artifacts themselves are downloaded as ZIP containers. This is normal. The installer `.exe` is inside the downloaded artifact.

The installer is currently unsigned. Windows SmartScreen can therefore warn that the publisher is unknown. Code signing is a later release/distribution step, not a compiler failure.
