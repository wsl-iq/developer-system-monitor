/**
 * Main Application Controller
 * Developer System Monitor
 */

// Translations dictionary
const translations = {
    en: {
        'dashboard': 'Dashboard',
        'processes': 'Processes',
        'cpu': 'CPU',
        'memory': 'Memory',
        'gpu': 'GPU',
        'disk': 'Disk',
        'network': 'Network',
        'services': 'Services',
        'hardware': 'Hardware',
        'logs': 'Logs',
        'settings': 'Settings',
        'cpu_usage': 'CPU Usage',
        'memory_usage': 'Memory',
        'gpu_usage': 'GPU Usage',
        'network_usage': 'Network',
        'processes_count': 'Processes',
        'uptime': 'Uptime',
        'download': 'Download',
        'upload': 'Upload',
        'frequency': 'Frequency',
        'temperature': 'Temperature',
        'total': 'Total',
        'used': 'Used',
        'available': 'Available',
        'free': 'Free',
        'search': 'Search',
        'refresh': 'Refresh',
        'terminate': 'Terminate',
        'suspend': 'Suspend',
        'resume': 'Resume',
        'start': 'Start',
        'stop': 'Stop',
        'restart': 'Restart',
        'connected': 'Connected',
        'disconnected': 'Disconnected',
        'loading': 'Loading...',
        'no_data': 'No data available',
        'no_processes': 'No processes found',
        'no_services': 'No services found',
        'no_adapters': 'No network adapters found',
        'save': 'Save',
        'cancel': 'Cancel',
        'confirm': 'Confirm',
        'close': 'Close',
        'export': 'Export',
        'clear': 'Clear',
        'reset': 'Reset',
        'theme': 'Theme',
        'language': 'Language',
        'dark': 'Dark',
        'light': 'Light',
        'english': 'English',
        'arabic': 'العربية',
        'last_update': 'Last update',
        'system_monitor': 'System Monitor',
        'process_manager': 'Process Manager',
        'cpu_monitor': 'CPU Monitor',
        'memory_monitor': 'Memory Monitor',
        'gpu_monitor': 'GPU Monitor',
        'disk_monitor': 'Disk Monitor',
        'network_monitor': 'Network Monitor',
        'windows_services': 'Windows Services',
        'hardware_info': 'Hardware Information',
        'app_logs': 'Application Logs',
        'settings_page': 'Settings',
        'confirm_terminate': 'Are you sure you want to terminate process',
        'confirm_stop_service': 'Are you sure you want to stop service',
        'confirm_restart_service': 'Are you sure you want to restart service',
        'confirm_clear_logs': 'Are you sure you want to clear all logs?',
        'confirm_reset_settings': 'Are you sure you want to reset all settings to defaults?',
    },
    ar: {
        'dashboard': 'لوحة التحكم',
        'processes': 'العمليات',
        'cpu': 'المعالج',
        'memory': 'الذاكرة',
        'gpu': 'بطاقة الرسوميات',
        'disk': 'الأقراص',
        'network': 'الشبكة',
        'services': 'الخدمات',
        'hardware': 'العتاد',
        'logs': 'السجلات',
        'settings': 'الإعدادات',
        'cpu_usage': 'استخدام المعالج',
        'memory_usage': 'الذاكرة',
        'gpu_usage': 'استخدام الرسوميات',
        'network_usage': 'الشبكة',
        'processes_count': 'العمليات',
        'uptime': 'وقت التشغيل',
        'download': 'التنزيل',
        'upload': 'الرفع',
        'frequency': 'التردد',
        'temperature': 'الحرارة',
        'total': 'الإجمالي',
        'used': 'المستخدم',
        'available': 'المتاح',
        'free': 'المساحة الحرة',
        'search': 'بحث',
        'refresh': 'تحديث',
        'terminate': 'إنهاء',
        'suspend': 'تعليق',
        'resume': 'استئناف',
        'start': 'تشغيل',
        'stop': 'إيقاف',
        'restart': 'إعادة تشغيل',
        'connected': 'متصل',
        'disconnected': 'غير متصل',
        'loading': 'جاري التحميل...',
        'no_data': 'لا توجد بيانات',
        'no_processes': 'لا توجد عمليات',
        'no_services': 'لا توجد خدمات',
        'no_adapters': 'لا توجد محولات شبكة',
        'save': 'حفظ',
        'cancel': 'إلغاء',
        'confirm': 'تأكيد',
        'close': 'إغلاق',
        'export': 'تصدير',
        'clear': 'مسح',
        'reset': 'إعادة تعيين',
        'theme': 'المظهر',
        'language': 'اللغة',
        'dark': 'داكن',
        'light': 'فاتح',
        'english': 'English',
        'arabic': 'العربية',
        'last_update': 'آخر تحديث',
        'system_monitor': 'مراقب النظام',
        'process_manager': 'مدير العمليات',
        'cpu_monitor': 'مراقب المعالج',
        'memory_monitor': 'مراقب الذاكرة',
        'gpu_monitor': 'مراقب الرسوميات',
        'disk_monitor': 'مراقب الأقراص',
        'network_monitor': 'مراقب الشبكة',
        'windows_services': 'خدمات Windows',
        'hardware_info': 'معلومات العتاد',
        'app_logs': 'سجلات التطبيق',
        'settings_page': 'الإعدادات',
        'confirm_terminate': 'هل أنت متأكد من إنهاء العملية',
        'confirm_stop_service': 'هل أنت متأكد من إيقاف الخدمة',
        'confirm_restart_service': 'هل أنت متأكد من إعادة تشغيل الخدمة',
        'confirm_clear_logs': 'هل أنت متأكد من مسح جميع السجلات؟',
        'confirm_reset_settings': 'هل أنت متأكد من إعادة تعيين جميع الإعدادات؟',
    }
};

class App {
    constructor() {
        this.currentPage = 'dashboard';
        this.theme = localStorage.getItem('dsm_theme') || 'dark';
        this.language = localStorage.getItem('dsm_language') || 'en';
        this.sidebarOpen = true;
        this.connected = false;
        
        this.pages = [
            'dashboard', 'processes', 'cpu', 'memory', 'gpu',
            'disk', 'network', 'services', 'hardware', 'logs', 'settings'
        ];
        
        this.pageTitles = {
            'dashboard': 'dashboard',
            'processes': 'process_manager',
            'cpu': 'cpu_monitor',
            'memory': 'memory_monitor',
            'gpu': 'gpu_monitor',
            'disk': 'disk_monitor',
            'network': 'network_monitor',
            'services': 'windows_services',
            'hardware': 'hardware_info',
            'logs': 'app_logs',
            'settings': 'settings_page'
        };
        
        this.initialize();
    }
    
    initialize() {
        this.applyTheme(this.theme);
        this.applyLanguage(this.language);
        this.setupNavigation();
        this.setupEventHandlers();
        this.setupWebSocket();
        this.navigateTo('dashboard');
        
        console.log('Developer System Monitor initialized');
    }
    
    translate(key) {
        const lang = translations[this.language] || translations.en;
        const fallback = translations.en || {};

        // Check if the key exists in the selected language
        if (lang[key] !== undefined) {
            return lang[key];
        }

        // If not found, use English
        if (fallback[key] !== undefined) {
            return fallback[key];
        }

        // If not found in any language, return the key itself
        return key;
    }
    
    setupNavigation() {
        const navLinks = document.querySelectorAll('.nav-link');
        
        navLinks.forEach(link => {
            link.addEventListener('click', (event) => {
                event.preventDefault();
                const page = link.getAttribute('data-page');
                this.navigateTo(page);
                
                // Close sidebar on mobile
                if (window.innerWidth <= 768) {
                    this.toggleSidebar(false);
                }
            });
        });
    }
    
    setupEventHandlers() {
        // Sidebar toggle
        const sidebarToggle = document.getElementById('sidebar-toggle');
        if (sidebarToggle) {
            sidebarToggle.addEventListener('click', () => {
                this.toggleSidebar();
            });
        }
        
        // Theme toggle
        const themeToggle = document.getElementById('theme-toggle');
        if (themeToggle) {
            themeToggle.addEventListener('click', () => {
                this.theme = this.theme === 'dark' ? 'light' : 'dark';
                this.applyTheme(this.theme);
                localStorage.setItem('dsm_theme', this.theme);
            });
        }
        
        // Refresh button
        const refreshButton = document.getElementById('refresh-button');
        if (refreshButton) {
            refreshButton.addEventListener('click', () => {
                this.refreshCurrentPage();
            });
        }
        
        // Language select
        const languageSelect = document.getElementById('language-select');
        if (languageSelect) {
            languageSelect.value = this.language;
            languageSelect.addEventListener('change', (event) => {
                this.language = event.target.value;
                this.applyLanguage(this.language);
                localStorage.setItem('dsm_language', this.language);
            });
        }
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (event) => {
            // F5 to refresh
            if (event.key === 'F5') {
                event.preventDefault();
                this.refreshCurrentPage();
            }
            
            // Ctrl+T to toggle theme
            if (event.ctrlKey && event.key === 't') {
                event.preventDefault();
                this.theme = this.theme === 'dark' ? 'light' : 'dark';
                this.applyTheme(this.theme);
                localStorage.setItem('dsm_theme', this.theme);
            }
            
            // Escape to close modal
            if (event.key === 'Escape') {
                this.closeModal();
            }
        });
    }
    
    setupWebSocket() {
        if (typeof wsClient === 'undefined') return;
        
        wsClient.on('connected', () => {
            this.connected = true;
            this.updateConnectionStatus(true);
        });
        
        wsClient.on('disconnected', () => {
            this.connected = false;
            this.updateConnectionStatus(false);
        });
        
        wsClient.on('metrics', (data) => {
            this.handleMetricsUpdate(data);
        });
        
        wsClient.connect();
    }
    
    navigateTo(page) {
        if (!this.pages.includes(page)) {
            console.error(`Unknown page: ${page}`);
            return;
        }
        
        this.currentPage = page;
        
        // Update navigation
        document.querySelectorAll('.nav-link').forEach(link => {
            const linkPage = link.getAttribute('data-page');
            link.classList.toggle('active', linkPage === page);
        });
        
        // Update pages
        document.querySelectorAll('.page').forEach(pageElement => {
            pageElement.classList.remove('active');
        });
        
        const targetPage = document.getElementById(`page-${page}`);
        if (targetPage) {
            targetPage.classList.add('active');
        }
        
        // Update title
        const titleElement = document.getElementById('page-title');
        if (titleElement) {
            const titleKey = this.pageTitles[page] || page;
            titleElement.textContent = this.translate(titleKey);
        }
        
        // Load page data
        this.loadPageData(page);
    }
    
    loadPageData(page) {
        switch (page) {
            case 'dashboard':
                if (typeof Dashboard !== 'undefined') Dashboard.load();
                break;
            case 'processes':
                if (typeof ProcessManager !== 'undefined') ProcessManager.load();
                break;
            case 'cpu':
                if (typeof SystemViewer !== 'undefined') SystemViewer.loadCPU();
                break;
            case 'memory':
                if (typeof SystemViewer !== 'undefined') SystemViewer.loadMemory();
                break;
            case 'gpu':
                if (typeof SystemViewer !== 'undefined') SystemViewer.loadGPU();
                break;
            case 'disk':
                if (typeof SystemViewer !== 'undefined') SystemViewer.loadDisk();
                break;
            case 'network':
                if (typeof NetworkViewer !== 'undefined') NetworkViewer.load();
                break;
            case 'services':
                if (typeof ServicesManager !== 'undefined') ServicesManager.load();
                break;
            case 'hardware':
                if (typeof HardwareViewer !== 'undefined') HardwareViewer.load();
                break;
            case 'logs':
                if (typeof LogViewer !== 'undefined') LogViewer.load();
                break;
            case 'settings':
                if (typeof SettingsManager !== 'undefined') SettingsManager.load();
                break;
        }
    }
    
    refreshCurrentPage() {
        this.loadPageData(this.currentPage);
        
        // Show refresh animation
        const refreshButton = document.getElementById('refresh-button');
        if (refreshButton) {
            refreshButton.classList.add('spinning');
            setTimeout(() => {
                refreshButton.classList.remove('spinning');
            }, 1000);
        }
    }
    
    handleMetricsUpdate(data) {
        // Dispatch event for pages
        const event = new CustomEvent('metrics-update', { detail: data });
        document.dispatchEvent(event);
        
        // Update last update time
        const timeElement = document.getElementById('last-update-time');
        if (timeElement && data.timestamp) {
            const date = new Date(data.timestamp);
            timeElement.textContent = date.toLocaleTimeString();
        }
    }
    
    applyTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        
        const themeToggle = document.getElementById('theme-toggle');
        if (themeToggle) {
            const icon = themeToggle.querySelector('i');
            if (icon) {
                icon.className = theme === 'dark' ? 'fas fa-sun' : 'fas fa-moon';
            }
        }
    }
    
    applyLanguage(language) {
        this.language = language || 'en';

        const dir = this.language === 'ar' ? 'rtl' : 'ltr';
        document.documentElement.setAttribute('lang', this.language);
        document.documentElement.setAttribute('dir', dir);

        // Update data-i18n
        document.querySelectorAll('[data-i18n]').forEach(element => {
            const key = element.getAttribute('data-i18n');
            const text = this.translate(key);
            if (text) {
                element.textContent = text;
            }
        });

        // Update placeholders
        document.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
            const key = element.getAttribute('data-i18n-placeholder');
            const text = this.translate(key);
            if (text) {
                element.placeholder = text;
            }
        });

        // Update titles
        document.querySelectorAll('[data-i18n-title]').forEach(element => {
            const key = element.getAttribute('data-i18n-title');
            const text = this.translate(key);
            if (text) {
                element.title = text;
            }
        });

        this.updatePageTitle();
        this.updateConnectionStatus(this.connected);

        const logoText = document.querySelector('.logo-text span');
        if (logoText) {
            logoText.textContent = this.translate('system_monitor');
        }

        console.log(`Language set to: ${this.language}, Direction: ${dir}`);
    }

    updatePageTitle() {
        const titleElement = document.getElementById('page-title');
        if (titleElement) {
            const titleKey = this.pageTitles[this.currentPage] || this.currentPage;
            titleElement.textContent = this.translate(titleKey);
        }
    }
    
    updateConnectionStatus(connected) {
        this.connected = connected;
        const dot = document.querySelector('.status-dot');
        const text = document.getElementById('connection-text');
        
        if (dot && text) {
            if (connected) {
                dot.classList.add('connected');
                text.textContent = this.translate('connected');
            } else {
                dot.classList.remove('connected');
                text.textContent = this.translate('disconnected');
            }
        }
    }
    
    toggleSidebar(open = null) {
        const sidebar = document.getElementById('sidebar');
        const mainContent = document.getElementById('main-content');
        
        if (sidebar && mainContent) {
            if (open === null) {
                this.sidebarOpen = !this.sidebarOpen;
            } else {
                this.sidebarOpen = open;
            }
            
            if (window.innerWidth <= 768) {
                sidebar.classList.toggle('open', this.sidebarOpen);
            } else {
                sidebar.classList.toggle('collapsed', !this.sidebarOpen);
                mainContent.classList.toggle('expanded', !this.sidebarOpen);
            }
        }
    }
    
    showNotification(message, type = 'info') {
        const container = document.getElementById('notifications');
        if (!container) return;
        
        const icons = {
            'info': 'fa-info-circle',
            'success': 'fa-check-circle',
            'warning': 'fa-exclamation-triangle',
            'error': 'fa-times-circle'
        };
        
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.innerHTML = `
            <i class="fas ${icons[type] || icons.info}"></i>
            <span>${message}</span>
            <button class="notification-close" onclick="this.parentElement.remove()">
                <i class="fas fa-times"></i>
            </button>
        `;
        
        container.appendChild(notification);
        
        setTimeout(() => {
            notification.style.opacity = '0';
            notification.style.transition = 'opacity 0.3s';
            setTimeout(() => notification.remove(), 300);
        }, 5000);
    }
    
    showConfirmation(message, titleKey = 'confirm') {
        return new Promise((resolve) => {
            const overlay = document.getElementById('modal-overlay');
            const modalContent = document.getElementById('modal-content');
            
            if (!overlay || !modalContent) return resolve(false);
            
            modalContent.innerHTML = `
                <h3 style="margin-bottom: 15px; color: var(--danger);">
                    <i class="fas fa-exclamation-triangle"></i> ${this.translate(titleKey)}
                </h3>
                <p style="margin-bottom: 20px;">${message}</p>
                <div style="display: flex; gap: 10px; justify-content: flex-end;">
                    <button class="btn btn-secondary" id="modal-cancel">
                        ${this.translate('cancel')}
                    </button>
                    <button class="btn btn-danger" id="modal-confirm">
                        ${this.translate('confirm')}
                    </button>
                </div>
            `;
            
            overlay.style.display = 'flex';
            
            modalContent.querySelector('#modal-cancel').addEventListener('click', () => {
                overlay.style.display = 'none';
                resolve(false);
            });
            
            modalContent.querySelector('#modal-confirm').addEventListener('click', () => {
                overlay.style.display = 'none';
                resolve(true);
            });
        });
    }
    
    showModal(content) {
        const overlay = document.getElementById('modal-overlay');
        const modalContent = document.getElementById('modal-content');
        
        if (!overlay || !modalContent) return;
        
        modalContent.innerHTML = content;
        overlay.style.display = 'flex';
        
        overlay.addEventListener('click', (event) => {
            if (event.target === overlay) {
                overlay.style.display = 'none';
            }
        });
    }
    
    closeModal() {
        const overlay = document.getElementById('modal-overlay');
        if (overlay) {
            overlay.style.display = 'none';
        }
    }
    
    formatBytes(bytes) {
        if (bytes === 0 || bytes === null || bytes === undefined) return '0 B';
        
        const units = this.language === 'ar' ? 
            ['ب', 'ك.ب', 'م.ب', 'ج.ب', 'ت.ب'] : 
            ['B', 'KB', 'MB', 'GB', 'TB'];
        
        const index = Math.floor(Math.log(Math.abs(bytes)) / Math.log(1024));
        
        if (index >= units.length) return bytes + (this.language === 'ar' ? ' ب' : ' B');
        
        return `${(bytes / Math.pow(1024, index)).toFixed(2)} ${units[index]}`;
    }
    
    formatUptime(seconds) {
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        if (this.language === 'ar') {
            if (days > 0) return `${days} يوم ${hours} ساعة ${minutes} دقيقة`;
            if (hours > 0) return `${hours} ساعة ${minutes} دقيقة`;
            return `${minutes} دقيقة`;
        }
        
        if (days > 0) return `${days}d ${hours}h ${minutes}m`;
        if (hours > 0) return `${hours}h ${minutes}m`;
        return `${minutes}m`;
    }
}

// Initialize app when DOM is ready
let app;
document.addEventListener('DOMContentLoaded', () => {
    app = new App();
});

// Export
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { App };
}
