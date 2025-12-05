#import "@local/oly:1.0.0": *
#set text(lang: "${language}")
#show: setup.with(
  title: "${title}",
  author: "${author}",
  date: datetime.today().display("[day] [month repr:long] [year]"),
  maketitle: false,
)

// main content
