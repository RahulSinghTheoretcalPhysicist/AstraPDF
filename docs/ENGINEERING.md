# Engineering Notes

## PDF page space
Qt PDF returns page sizes in points. One point is 1/72 inch.

At scale `s`: `Wr = Wp*s`, `Hr = Hp*s`.

## Facing-page normalization
For heights `HL`, `HR`, zoom `z`:
`commonHeight = max(HL,HR)*z`
`sL = commonHeight/HL`
`sR = commonHeight/HR`

Widths:
`WL' = WL*sL`
`WR' = WR*sR`

Positions:
left: `[x0,y0,WL',commonHeight]`
right: `[x0+WL'+gap,y0,WR',commonHeight]`

## Hit testing
For mouse `(Mx,My)` and page rectangle origin `(x0,y0)`:
`xp=(Mx-x0)/s`
`yp=(My-y0)/s`

Qt PDF selection geometry uses upper-left page origin in points, so no Y inversion is required for `QPdfDocument::getSelection()`.

## COS inspector
The first low-level inspector deliberately reads the file bytes itself and checks:
`%PDF-1.x`, `startxref`, final `%%EOF`, and a classic trailer snippet.
Next stages add classic xref entry parsing, xref streams, indirect objects, stream filters, object streams, `/Root` resolution, `/Pages` traversal and `/Prev` incremental-update chains.
