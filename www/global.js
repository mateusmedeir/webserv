let toastTimer = null

function showToast(message, type = 'info', duration = 3000) {
  const toast = document.getElementById('toast')

  if (toastTimer) {
    clearTimeout(toastTimer)
    toastTimer = null
  }

  toast.textContent = message
  toast.style.visibility = 'visible'

  toast.style.backgroundColor =
    type === 'success'
      ? 'var(--accent)'
      : type === 'error'
      ? 'var(--error)'
      : '#333'

  toastTimer = setTimeout(() => {
    toast.style.visibility = 'hidden'
    toast.textContent = ''
    toastTimer = null
  }, duration)
}

function setCookie(name, value, days) {
    let expires = "";
    if (days) {
        const date = new Date();
        date.setTime(date.getTime() + (days*24*60*60*1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + (value || "")  + expires + "; path=/";
}

function getCookie(name) {
    const nameEQ = name + "=";
    const ca = document.cookie.split(';');
    for(let i=0;i < ca.length;i++) {
        let c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
}

function getTheme() {
    return getCookie('theme') || 'dark';
}

function toggleTheme() {
    const currentTheme = getTheme();
    const newTheme = currentTheme === 'light' ? 'dark' : 'light';
    setTheme(newTheme);
}

function setTheme(theme) {
    if (theme === 'light') {
        document.body.classList.add('light-theme');
    } else {
        document.body.classList.remove('light-theme');
    }
    setCookie('theme', theme, 365);
}

function initTheme() {
    const savedTheme = getCookie('theme') || 'dark';
    setTheme(savedTheme);

    const lightButton = document.getElementById('theme-light');
    const darkButton = document.getElementById('theme-dark');

    if (lightButton && darkButton) {
        lightButton.addEventListener('click', () => setTheme('light'));
        darkButton.addEventListener('click', () => setTheme('dark'));
    }
}

document.addEventListener('DOMContentLoaded', initTheme);