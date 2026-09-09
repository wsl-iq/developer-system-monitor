/**
 * Log Viewer Component
 */

class LogViewer {
    static logs = [];
    static searchQuery = '';
    static filterLevel = 'all';
    static initialized = false;

    static initialize() {
        if (this.initialized) return;
        this.setupEventListeners();
        this.initialized = true;
    }

    static setupEventListeners() {
        const clearButton = document.getElementById('clear-logs');
        if (clearButton) {
            clearButton.addEventListener('click', () => this.clearLogs());
        }

        const exportButton = document.getElementById('export-logs');
        if (exportButton) {
            exportButton.addEventListener('click', () => this.exportLogs());
        }
    }

    static async load() {
        this.initialize();

        try {
            const logs = await api.getLogs();
            this.logs = Array.isArray(logs) ? logs : [];
            this.render();
        } catch (error) {
            console.error('Failed to load logs:', error);
        }
    }

    static render() {
        const container = document.getElementById('log-container');
        if (!container) return;

        let logs = [...this.logs];

        if (this.searchQuery) {
            logs = logs.filter(log => {
                return (log.message || '').toLowerCase().includes(this.searchQuery) ||
                       (log.level || '').toLowerCase().includes(this.searchQuery);
            });
        }

        if (this.filterLevel !== 'all') {
            logs = logs.filter(log => (log.level || '').toLowerCase() === this.filterLevel);
        }

        if (logs.length === 0) {
            container.innerHTML = `
                <div class="empty-state">
                    <i class="fas fa-scroll"></i>
                    <p>No logs available</p>
                </div>
            `;
            return;
        }

        container.innerHTML = logs.map(log => {
            const level = (log.level || 'INFO').toLowerCase();
            const levelClass = this.getLevelClass(level);
            const levelIcon = this.getLevelIcon(level);
            const timestamp = log.timestamp || new Date().toLocaleTimeString();

            return `
                <div class="log-entry">
                    <span class="log-timestamp">${timestamp}</span>
                    <span class="log-level log-${levelClass}">
                        <i class="fas ${levelIcon}"></i> ${level.toUpperCase()}
                    </span>
                    <span class="log-message">${this.escapeHtml(log.message || '')}</span>
                </div>
            `;
        }).join('');

        // Auto-scroll to bottom
        container.scrollTop = container.scrollHeight;
    }

    static async clearLogs() {
        const confirmed = await app.showConfirmation(
            'Are you sure you want to clear all logs?',
            'Clear Logs'
        );

        if (!confirmed) return;

        try {
            await api.clearLogs();
            this.logs = [];
            this.render();
            app.showNotification('Logs cleared', 'success');
        } catch (error) {
            console.error('Failed to clear logs:', error);
            app.showNotification('Failed to clear logs', 'error');
        }
    }

    static exportLogs() {
        const content = this.logs.map(log => 
            `[${log.timestamp || ''}] [${log.level || 'INFO'}] ${log.message || ''}`
        ).join('\n');

        const blob = new Blob([content], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = `dsm_logs_${new Date().toISOString().replace(/[:.]/g, '-')}.txt`;
        link.click();
        URL.revokeObjectURL(url);

        app.showNotification('Logs exported', 'success');
    }

    static getLevelClass(level) {
        switch (level) {
            case 'error': return 'error';
            case 'warning': return 'warning';
            case 'info': return 'info';
            case 'debug': return 'debug';
            default: return 'info';
        }
    }

    static getLevelIcon(level) {
        switch (level) {
            case 'error': return 'fa-times-circle';
            case 'warning': return 'fa-exclamation-triangle';
            case 'info': return 'fa-info-circle';
            case 'debug': return 'fa-bug';
            default: return 'fa-info-circle';
        }
    }

    static escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { LogViewer };
}