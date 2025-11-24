// theorems
#import "theorems.typ": *
#show: thmrules.with(qed-symbol: $square$)

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
  ),
)
#let get_name(name) = {
  return context names.at(text.lang).at(name)
}

// environments
#let proof = thmproof("proof", get_name("proof"))
#let solution = thmproof("solution", get_name("solution"))
#let env(env_name, newline: false, style: "normal") = thmenv(
  env_name,
  none,
  none,
  (name, number, body, color: black) => [
    #let header = [#get_name(env_name)]
    #if number != [] { header += [ #number] }
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
#let title(title) = {
  v(1em)
  set align(center)
  set block(spacing: 2em)
  block(text(
    size: 1.5em,
    weight: "bold",
    smallcaps(title),
  ))
}
