index.html: index.md header.html footer.html
	cat header.html > index.html
	marked index.md >> index.html
	cat footer.html >> index.html
