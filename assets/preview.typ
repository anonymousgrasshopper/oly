#import "@local/oly:1.0.0": *
#set text(lang: "${language}")
#show: setup.with(
  document_title: if "${title}".len() == 0 { "${source}" } else { "${title}" },
  author: "${author}",
  date: datetime.today().display("[day] [month repr:long] [year]"),
  maketitle: ("${title}".len() != 0),
)

// main content
#include "solution.typ"
