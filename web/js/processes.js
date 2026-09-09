/**
 * Process Manager Component
 */

class ProcessManager {
    static processes = [];
    static sortField = 'name';
    static sortOrder = 'asc';
    static searchQuery = '';
    static initialized = false;
    static refreshInterval = null;

    static initialize() {
        if (this.initialized) return;
        this.setupEventListeners();
        this.initialized = true;
    }

    static setupEventListeners() {
        const searchInput = document.getElementById('process-search');
        if (searchInput) {
            searchInput.addEventListener('input', (event) => {
                this.searchQuery = event.target.value.toLowerCase();
                this.render();
            });
        }

        const refreshButton = document.getElementById('process-refresh');
        if (refreshButton) {
            refreshButton.addEventListener('click', () => this.load());
        }

        // Sort headers
        document.querySelectorAll('#process-table th[data-sort]').forEach(header => {
            header.addEventListener('click', () => {
                const field = header.getAttribute('data-sort');
                this.toggleSort(field);
            });
        });
    }

    static async load() {
        this.initialize();

        try {
            if (typeof api === 'undefined') return;

            const processes = await api.getProcesses();
            this.processes = Array.isArray(processes) ? processes : [];
            this.render();

            // Update stats
            await this.updateStats();

            // Auto refresh every 5 seconds
            this.startAutoRefresh();
        } catch (error) {
            console.error('Failed to load processes:', error);
            this.showError('Failed to load process list');
        }
    }

    static startAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
        }

        this.refreshInterval = setInterval(async () => {
            try {
                const processes = await api.getProcesses();
                this.processes = Array.isArray(processes) ? processes : [];
                this.render();
                await this.updateStats();
            } catch (error) {
                // Silent fail for auto-refresh
            }
        }, 5000);
    }

    static async updateStats() {
        try {
            const stats = await api.getProcessStats();
            if (stats) {
                const countElement = document.getElementById('process-count-value');
                if (countElement && stats.total_processes) {
                    countElement.textContent = stats.total_processes;
                }

                const memElement = document.getElementById('process-memory-total');
                if (memElement && stats.total_memory_bytes) {
                    memElement.textContent = `${app.formatBytes(stats.total_memory_bytes)} used`;
                }
            }
        } catch (error) {
            // Silent fail
        }
    }

    static toggleSort(field) {
        if (this.sortField === field) {
            this.sortOrder = this.sortOrder === 'asc' ? 'desc' : 'asc';
        } else {
            this.sortField = field;
            this.sortOrder = 'asc';
        }

        // Update sort indicators
        document.querySelectorAll('#process-table th[data-sort]').forEach(header => {
            header.classList.remove('sort-asc', 'sort-desc');
            if (header.getAttribute('data-sort') === field) {
                header.classList.add(this.sortOrder === 'asc' ? 'sort-asc' : 'sort-desc');
            }
        });

        this.render();
    }

    static render() {
        const tableBody = document.getElementById('process-table-body');
        if (!tableBody) return;

        let processes = [...this.processes];

        // Apply search
        if (this.searchQuery) {
            processes = processes.filter(process => {
                return (process.name || '').toLowerCase().includes(this.searchQuery) ||
                       String(process.pid || '').includes(this.searchQuery);
            });
        }

        // Apply sort
        processes.sort((a, b) => {
            let valueA = a[this.sortField] || '';
            let valueB = b[this.sortField] || '';

            if (typeof valueA === 'string') {
                return this.sortOrder === 'asc' 
                    ? valueA.localeCompare(valueB)
                    : valueB.localeCompare(valueA);
            }
            return this.sortOrder === 'asc' ? valueA - valueB : valueB - valueA;
        });

        if (processes.length === 0) {
            const emptyMessage = typeof app !== 'undefined' && app.translate
                ? app.translate('no_processes')
                : 'No processes found';

            tableBody.innerHTML = `
                <tr>
                    <td colspan="6" class="empty-state">
                        <i class="fas fa-search"></i>
                        <p>${emptyMessage}</p>
                    </td>
                </tr>
            `;
            return;
        }

        tableBody.innerHTML = processes.map(process => `
            <tr>
                <td>
                    <strong>${this.escapeHtml(process.name || 'Unknown')}</strong>
                </td>
                <td>${process.pid || 0}</td>
                <td>${(process.cpu_usage_percent || 0).toFixed(1)}%</td>
                <td>${app.formatBytes(process.memory_usage_bytes || 0)}</td>
                <td>${process.thread_count || 0}</td>
                <td>
                    <div class="action-buttons">
                        <button class="btn btn-sm btn-info" 
                                onclick="ProcessManager.showDetails(${process.pid})" 
                                title="View details">
                            <i class="fas fa-info-circle"></i>
                        </button>
                        <button class="btn btn-sm btn-danger" 
                                onclick="ProcessManager.terminate(${process.pid})" 
                                title="Terminate process">
                            <i class="fas fa-power-off"></i>
                        </button>
                        <button class="btn btn-sm btn-warning" 
                                onclick="ProcessManager.suspend(${process.pid})" 
                                title="Suspend process">
                            <i class="fas fa-pause"></i>
                        </button>
                    </div>
                </td>
            </tr>
        `).join('');
    }

    static async showDetails(pid) {
        try {
            const process = await api.getProcess(pid);
            if (!process) return;

            const content = `
                <div class="process-details-modal">
                    <h3><i class="fas fa-microchip"></i> ${this.escapeHtml(process.name || 'Process')}</h3>
                    <div class="process-details-grid">
                        <div class="detail-item">
                            <label>PID</label>
                            <span>${process.pid}</span>
                        </div>
                        <div class="detail-item">
                            <label>Parent PID</label>
                            <span>${process.parent_pid || 'N/A'}</span>
                        </div>
                        <div class="detail-item">
                            <label>CPU Usage</label>
                            <span>${(process.cpu_usage_percent || 0).toFixed(2)}%</span>
                        </div>
                        <div class="detail-item">
                            <label>Memory</label>
                            <span>${app.formatBytes(process.memory_usage_bytes || 0)}</span>
                        </div>
                        <div class="detail-item">
                            <label>Threads</label>
                            <span>${process.thread_count || 0}</span>
                        </div>
                        <div class="detail-item">
                            <label>User</label>
                            <span>${this.escapeHtml(process.user_name || 'Unknown')}</span>
                        </div>
                        <div class="detail-item" style="grid-column: 1/-1;">
                            <label>Executable Path</label>
                            <span>${this.escapeHtml(process.executable_path || 'Unknown')}</span>
                        </div>
                    </div>
                </div>
            `;

            app.showModal(content);
        } catch (error) {
            console.error('Failed to get process details:', error);
        }
    }

    static async terminate(pid) {
        const confirmed = await app.showConfirmation(
            `Are you sure you want to terminate process ${pid}? This may cause data loss.`,
            'Terminate Process'
        );

        if (!confirmed) return;

        try {
            const result = await api.terminateProcess(pid);
            app.showNotification(`Process ${pid} terminated`, 'success');
            this.load();
        } catch (error) {
            console.error('Failed to terminate process:', error);
            app.showNotification(`Failed to terminate process ${pid}`, 'error');
        }
    }

    static async suspend(pid) {
        const confirmed = await app.showConfirmation(
            `Are you sure you want to suspend process ${pid}?`,
            'Suspend Process'
        );

        if (!confirmed) return;

        try {
            const result = await api.suspendProcess(pid);
            app.showNotification(`Process ${pid} suspended`, 'warning');
            this.load();
        } catch (error) {
            console.error('Failed to suspend process:', error);
            app.showNotification(`Failed to suspend process ${pid}`, 'error');
        }
    }

    static escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    static showError(message) {
        app.showNotification(message, 'error');
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { ProcessManager };
}