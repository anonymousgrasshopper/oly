// theorems
#import "theorems.typ": *

// language
#let names = (
  "fr": (
    "theorem": "Théorème",
    "corollary": "Corollaire",
    "lemma": "Lemme",
    "proof": "Preuve",
    "proposition": "Proposition",
    "definition": "Définition",
    "exercise": "Exercice",
    "example": "Exemple",
    "remark": "Remarque",
    "solution": "Solution",
    "conjecture": "Conjecture",
  ),
  "en": (
    "theorem": "Theorem",
    "corollary": "Corollary",
    "lemma": "Lemma",
    "proof": "Proof",
    "proposition": "Proposition",
    "definition": "Definition",
    "exercise": "Exercise",
    "example": "Example",
    "remark": "Remark",
    "solution": "Solution",
    "conjecture": "Conjecture",
  ),
)
#let get_name(name) = {
  return context names.at(text.lang).at(name)
}

// environments
#let proof = thmproof("proof", get_name("proof"))
#let solution = thmproof("solution", get_name("solution"))
#let env(env_name, newline: false, style: "normal", numbering: true) = thmenv(
  env_name,
  none,
  none,
  (name, number, body, color: black) => [
    #let header = [#get_name(env_name)]
    #if numbering and number != none { header += [ #number] }
    #if not newline { header += [:] }
    #text(weight: "bold", style: style)[#header]
    #if type(name) == str or type(name) == content {
      text()[ (#name)]
    }
    #if newline {
      text()[\ ]
    }
    #h(0.2em)
    #body
  ],
)

#let lemma = env("lemma", style: "italic")
#let exercise = env("exercise", newline: true)
#let definition = env("definition")
#let example = env("example")
#let remark = env("remark")
#let conjecture = env("conjecture", style: "italic")

#let _lemma = env("lemma", style: "italic", numbering: false)
#let _exercise = env("exercise", newline: true, numbering: false)
#let _definition = env("definition", numbering: false)
#let _example = env("example", numbering: false)
#let _remark = env("remark", numbering: false)
#let _conjecture = env("conjecture", style: "italic", numbering: false)

#let theorem = thmbox(
  "theorem",
  get_name("theorem"),
  fill: rgb("#e8e8f8"),
).with(numbering: none)
#let corollary = thmbox(
  "corollary",
  get_name("corollary"),
  fill: rgb("#e8e8f8"),
).with(numbering: none)
#let proposition = thmbox(
  "proposition",
  get_name("proposition"),
  fill: rgb("#e8e8f8"),
).with(numbering: none)

#let eqn(it) = {
  set math.equation(numbering: "(1)")
  it
}

// symbols
#let iff = sym.arrow.l.r.double.long
#let pm = sym.plus.minus
#let mp = sym.minus.plus
#let int = sym.integral
#let oint = sym.integral.cont
#let iint = sym.integral.double
#let oiint = sym.integral.surf
#let iiint = sym.integral.triple
#let oiiint = sym.integral.vol

#let pgcd = math.op("pgcd")
#let ppcm = math.op("ppcm")
#let ord = math.op("ord")

#let pmod(x) = $space (mod #x)$
#let bf(x) = $bold(upright(#x))$
#let dbbracket(lhs, rhs) = {
  if type(lhs) == array {
    lhs = lhs.join()
  }
  if type(rhs) == array {
    rhs = rhs.join()
  }
  math.lr[$⟦ lhs ; rhs ⟧$]
}
#let proj(point) = {
  math.attach([$=$], t: [$#point$])
}


// commands
#let hrule = box(width: 100%, align(center)[#line(length: 95%, stroke: 0.8pt)])
#let Box = {
  h(1fr)
  text(size: 1.4em, $square$)
}
#let scr(it) = text(
  features: ("ss01",),
  box($cal(it)$),
)

// main setup
#let setup(title: none, author: none, date: none, maketitle: true, body) = {
  if (title != none) {
    set document(title: title)
  }
  if (author != none) {
    set document(author: author)
  }
  set page(
    paper: "a4",
    margin: auto,
    header: context {
      set text(size: 0.8em)
      set align(left)
      text(style: "normal", author)
      h(0.2em)
      sym.dash.em
      h(0.2em)
      text(style: "italic", date)
      h(1fr)
      text(weight: "bold", title)
      box(width: 100%, align(center)[#line(length: 100%, stroke: 0.7pt)])
    },
    numbering: "1",
  )
  set heading(
    numbering: (..nums) => {
      let numbers = nums.pos()
      if numbers.len() == 1 {
        numbering("I.", ..numbers)
      }
    },
  )
  set par(
    justify: true,
  )
  set text(
    font: "New Computer Modern",
    size: 11pt,
    fallback: false,
  )

  // Change quote display
  set quote(block: true)
  show quote: set pad(x: 2em, y: 0em)
  show quote: it => {
    set text(style: "italic")
    v(-1em)
    it
    v(-0.5em)
  }

  // Indent lists
  set enum(indent: 1em)
  set list(indent: 1em)

  // theorems
  show: thmrules.with(qed-symbol: $square$)

  if (maketitle and (type(title) == str and title.len() > 0)) {
    v(1em)
    set align(center)
    set block(spacing: 2em)
    block(text(
      size: 1.8em,
      weight: "bold",
      smallcaps(title),
    ))
    v(1em)
  }

  body
}
