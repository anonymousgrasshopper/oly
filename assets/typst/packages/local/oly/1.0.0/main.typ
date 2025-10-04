// theorems
#import "theorems.typ": *
#show: thmrules.with(qed-symbol: $square$)

// environments
#let lemma = thmenv(
  "Lemme",
  none,
  none,
  (name, number, body, color: black) => [
    #text(color)[_*Lemme #number#name:*_]
    #h(0.2em)
    #body
  ],
)
#let proof = thmproof("proof", "Preuve")

// shortcuts
#let iff = sym.arrow.l.r.double.long
#let pm = sym.plus.minus
#let mp = sym.minus.plus
#let int = sym.integral
#let oint = sym.integral.cont
#let iint = sym.integral.double
#let oiint = sym.integral.surf
#let iiint = sym.integral.triple
#let oiiint = sym.integral.vol

// commands
#let hrule = box(width: 100%, align(center)[#line(length: 95%, stroke: 0.8pt)])
#let Box = {
  h(1fr)
  text(size: 1.4em, $square$)
}
