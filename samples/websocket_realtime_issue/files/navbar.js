$(document).ready(function() {
  // Define your Handlebars template here
  var navbarTemplate = `
<nav class="navbar navbar-expand-lg navbar-light bg-light">
    <a class="navbar-brand" href="index.html">BalanceEsp</a>
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarNav" aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarNav">
        <ul class="navbar-nav">
            <li class="nav-item">
                <a class="nav-link" href="dev.html">Config</a>
            </li>
        </ul>
    </div>
</nav>
  `;

  // Compile the template
  var template = Handlebars.compile(navbarTemplate);

  // Render the navigation bar
  $("#navbar-frame").html(template());
});
