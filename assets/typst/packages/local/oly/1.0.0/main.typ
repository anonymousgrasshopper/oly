// theorems
#import "theorems.typ": *
#show: thmrules.with(qed-symbol: $square$)

// language
#let lang = state("lang", "english")
#let names = state("names", (
  "lemma": "Lemma",
  "theorem": "Theorem",
  "proof": "Proof",
))

#let language(new_lang) = {
  assert(
    type(new_lang) == str,
    message: "called language function from oly package with argument"
      + new_lang
      + "which doesn't have type string",
  )

  lang.update(new_lang)
  if new_lang == "french" {
    set text(lang: "fr")
    names.update(("lemma": "Lemme", "theorem": "Théorème", "proof": "Preuve"))
  } else {
    set text(lang: "eng")
    names.update(("lemma": "Lemma", "theorem": "Theorem", "proof": "Proof"))
  }
}

// environments
#let lemma = thmenv(
  context names.get().at("lemma"),
  none,
  none,
  (name, number, body, color: black) => [
    #text(color)[_*Lemme #number#name:*_]
    #h(0.2em)
    #body
  ],
)
#let proof = thmproof("proof", context names.get().at("proof"))
#let theorem = thmbox(
  "theorem",
  context names.get().at("theorem"),
  fill: rgb("#e8e8f8"),
).with(numbering: none)

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

#let dbbracket(lhs, rhs) = {
  if type(lhs) == array {
    lhs = lhs.join()
  }
  if type(rhs) == array {
    rhs = rhs.join()
  }
  math.lr[$⟦ lhs ; rhs ⟧$]
}

// commands
#let hrule = box(width: 100%, align(center)[#line(length: 95%, stroke: 0.8pt)])
#let Box = {
  h(1fr)
  text(size: 1.4em, $square$)
}
