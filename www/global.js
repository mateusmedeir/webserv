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

function setCookie(name, value, days) {
    let expires = "";
    if (days) {
        const date = new Date();
        date.setTime(date.getTime() + (days*24*60*60*1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + (value || "")  + expires + "; path=/";
}

function deleteCookie(name) {
    document.cookie = name + "=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
}

// Verifica se o servidor habilitou cookies (enviou session_id)
function areCookiesEnabled() {
    return getCookie('session_id') !== null;
}

function getTheme() {
    if (areCookiesEnabled()) {
        return getCookie('theme') || 'dark';
    }
    return 'dark'; // Default quando cookies desabilitados
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
    
    // Só salva o tema se o servidor habilitou cookies
    if (areCookiesEnabled()) {
        setCookie('theme', theme, 365);
    } else {
        // Remove o cookie de tema se existir (servidor desabilitou cookies)
        deleteCookie('theme');
    }
}

function initTheme() {
    const savedTheme = getTheme();
    setTheme(savedTheme);

    const lightButton = document.getElementById('theme-light');
    const darkButton = document.getElementById('theme-dark');

    if (lightButton && darkButton) {
        lightButton.addEventListener('click', () => setTheme('light'));
        darkButton.addEventListener('click', () => setTheme('dark'));
    }
}

document.addEventListener('DOMContentLoaded', initTheme);