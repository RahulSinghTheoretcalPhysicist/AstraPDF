# AstraPDF

Windows 11 x64 PDF reader prototype in C++20 + Qt 6 PDF.

## Implemented now
- PDF loading/rendering
- Single Page
- Continuous
- Facing Pages
- Continuous Facing
- Dual-page height normalization while preserving aspect ratio
- Mouse hit-testing: canvas pixels -> page points
- Text selection and copy
- Search and jump to match
- Highlight overlay
- Render cache
- Physical PDF/COS inspection: `%PDF-x.y`, `startxref`, `%%EOF`, trailer/tail
- GitHub Actions Windows x64 build
- Portable EXE package

## Important boundary
The highlight layer is currently an application overlay. It is not yet written back into the PDF as a native `/Annot` object. True object-level editing, incremental-update writing, OCR, xref-stream parsing and native annotation persistence are the next engine stages.

## GitHub-only compilation
You do not need to install Visual Studio, Qt or CMake locally.

1. Create a GitHub repository.
2. Upload every file/folder from this project into the repository root.
3. Commit.
4. Open **Actions**.
5. Open **Build Windows EXE**.
6. Click **Run workflow**.
7. When `windows-x64` is green, open the run.
8. Under **Artifacts**, download either **AstraPDF-Portable-Windows-x64** or **AstraPDF-Setup-Windows-x64**.
9. Use the portable ZIP for no-install use, or extract the setup artifact and run `AstraPDF-Setup-Windows-x64.exe` for normal Windows installation.

See `GITHUB_COMPILE_GUIDE.md` for the exact click-by-click procedure.

## Performance status

The current verified milestone renders pages on demand from the custom canvas, but rendering is still synchronous inside the paint path. This is acceptable for correctness testing, but it is **not yet the final lag-free architecture** required by the project specification.

The next performance milestone must move page rendering to `QPdfPageRenderer` or a dedicated worker/render queue, add cancellation/prioritization, and keep only visible/near-visible pages in the cache.
