const root = document.documentElement;
const toggle = document.querySelector("[data-theme-toggle]");
const storedTheme = localStorage.getItem("nextstars-portfolio-theme");

if (storedTheme === "dark" || storedTheme === "light") {
  root.dataset.theme = storedTheme;
}

toggle?.addEventListener("click", () => {
  const nextTheme = root.dataset.theme === "dark" ? "light" : "dark";
  root.dataset.theme = nextTheme;
  localStorage.setItem("nextstars-portfolio-theme", nextTheme);
});
