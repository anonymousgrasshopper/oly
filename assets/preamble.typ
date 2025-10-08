#let title = "${source}"
#let author = "${author}"
#let today = datetime.today().display("[day] [month repr:long] [year]")
// #let today = "${date}"

#set document(title: title)
#set document(author: author)
#set page(
  paper: "a4",
  margin: auto,
  header: context {
    set text(size: 0.8em)
    set align(left)
    text(style: "normal", author)
    h(0.2em)
    sym.dash.em
    h(0.2em)
    text(style: "italic", today)
    h(1fr)
    text(weight: "bold", title)
    box(width: 100%, align(center)[#line(length: 100%, stroke: 0.7pt)])
  },
  numbering: "1",
)

#set par(
  justify: true,
)
#set text(
  font: "New Computer Modern",
  lang: "fr",
  size: 11pt,
  fallback: false,
)

// Change quote display
#set quote(block: true)
#show quote: set pad(x: 2em, y: 0em)
#show quote: it => {
  set text(style: "italic")
  v(-1em)
  it
  v(-0.5em)
}

// Indent lists
#set enum(indent: 1em)
#set list(indent: 1em)

// Hyperlinks
#show link: it => {
  set text(fill: if (type(it.dest) == label) { colors.label } else {
    colors.hyperlink
  })
  it
}
#show ref: it => {
  link(it.target, it)
}

// theorems
#import "@local/oly:1.0.0": *
#show: thmrules.with(qed-symbol: $square$)
#language("${language}")

// main content
