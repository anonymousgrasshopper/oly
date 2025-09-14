#let title = "${source}"
#let author = "${author}"
#let today = datetime.today().display("[day] [month repr:long] [year]")

#set document(title: title)
#set document(author: author)
#set page(
  paper: "a4",
  margin: auto,
  header: context {
    set text(size:0.8em)
    set align(left)
    text(style:"normal", author)
    h(0.2em)
    sym.dash.em
    h(0.2em)
    text(style: "italic", today)
    h(1fr)
    text(weight:"bold", title)
    box(width: 100%, align(center)[#line(length: 100%, stroke: 0.7pt)])
  },
  numbering: "1",
)

#set par(
  justify: true
)
#set text(
  // font: fonts.text,
  size: 11pt,
  fallback: false,
)

// For bold elements, use sans font
// show strong: set text(font:fonts.sans, size: 0.9em)

// Theorem environments
// show: thm-rules.with(qed-symbol: $square$)

// Change quote display
#set quote(block: true)
#show quote: set pad(x:2em, y:0em)
#show quote: it => {
  set text(style:"italic")
  v(-1em)
  it
  v(-0.5em)
}

// Indent lists
#set enum(indent: 1em)
#set list(indent: 1em)

// Hyperlinks
#show link: it => {
  set text(fill:
    if (type(it.dest) == label) { colors.label } else { colors.hyperlink }
  )
  it
}
#show ref: it => {
  link(it.target, it)
}

// main content
#include "solution.typ"
#h(1fr)
#text(size: 1.4em, math.square.stroked)
